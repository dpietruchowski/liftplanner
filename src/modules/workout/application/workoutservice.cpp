#include "workoutservice.h"
#include "async/timeprovider.h"
#include "modules/workout/domain/entities/workoutstatus.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/domain/repositories/workoutrepository.h"
#include <QHash>
#include <algorithm>

namespace
{

constexpr int previous_performance_window = 50;

bool sameExercise(const Exercise& a, const Exercise& b)
{
    if (a.hasDefinition() && b.hasDefinition())
        return a.definitionId() == b.definitionId();
    return a.name().compare(b.name(), Qt::CaseInsensitive) == 0;
}

bool hasCompletedSet(const Exercise& exercise)
{
    return std::any_of(exercise.sets().begin(), exercise.sets().end(),
                       [](const Set& set) { return set.completed(); });
}

}

WorkoutService::WorkoutService(WorkoutRepository& repository, QObject* worker)
    : Service(worker)
    , m_repository(repository)
{
}

Task<std::vector<Workout>> WorkoutService::loadPlannedWorkouts()
{
    return invoke([this] { return loadPlannedWorkoutsCore(); });
}

Task<void> WorkoutService::importPlannedWorkouts(const std::vector<Workout>& workouts)
{
    return invoke([this, workouts] { return importPlannedWorkoutsCore(workouts); });
}

Task<void> WorkoutService::removeAllPlannedWorkouts()
{
    return invoke([this] { return removeAllPlannedWorkoutsCore(); });
}

Task<std::vector<Workout>> WorkoutService::loadHistory(int limit)
{
    return invoke([this, limit] { return loadHistoryCore(limit); });
}

Task<void> WorkoutService::importHistory(const std::vector<Workout>& workouts)
{
    return invoke([this, workouts] { return importHistoryCore(workouts); });
}

Task<std::vector<WorkoutService::ExerciseFrequency>>
WorkoutService::topExercises(int topN, int recentWorkouts)
{
    return invoke([this, topN, recentWorkouts] { return topExercisesCore(topN, recentWorkouts); });
}

Task<WorkoutService::TrainingTotals> WorkoutService::recentTotals(int recentWorkouts)
{
    return invoke([this, recentWorkouts] { return recentTotalsCore(recentWorkouts); });
}

Task<std::vector<WorkoutService::PreviousPerformance>>
WorkoutService::previousPerformances(const Workout& workout)
{
    return invoke([this, workout] { return previousPerformancesCore(workout); });
}

Task<std::optional<Workout>> WorkoutService::findWorkout(int id)
{
    return invoke([this, id] { return findWorkoutCore(id); });
}

Task<int> WorkoutService::saveWorkout(const Workout& workout)
{
    return invoke([this, workout] { return saveWorkoutCore(workout); });
}

Task<bool> WorkoutService::deleteWorkout(int id)
{
    return invoke([this, id] { return deleteWorkoutCore(id); });
}

Result<std::vector<Workout>> WorkoutService::loadPlannedWorkoutsCore()
{
    WorkoutQuery query;
    query.whereStatus(WorkoutStatus::Planned);
    query.orderByPlannedTime(SortDirection::Ascending);
    return Result<std::vector<Workout>>::success(m_repository.findAll(query));
}

Result<void> WorkoutService::importPlannedWorkoutsCore(const std::vector<Workout>& workouts)
{
    removeAllPlannedWorkoutsCore();
    for (const auto& workout : workouts)
        m_repository.save(workout);
    return Result<void>::success();
}

Result<void> WorkoutService::removeAllPlannedWorkoutsCore()
{
    WorkoutQuery query;
    query.whereStatus(WorkoutStatus::Planned);
    m_repository.remove(query);
    return Result<void>::success();
}

Result<std::vector<Workout>> WorkoutService::loadHistoryCore(int limit)
{
    WorkoutQuery query;
    query.whereStatus(WorkoutStatus::Ended);
    query.orderByStartedTime(SortDirection::Descending);
    if (limit > 0)
        query.withLimit(limit);
    return Result<std::vector<Workout>>::success(m_repository.findAll(query));
}

Result<void> WorkoutService::importHistoryCore(const std::vector<Workout>& workouts)
{
    for (auto workout : workouts)
    {
        workout.setStatus(WorkoutStatus::Ended);
        if (!workout.startedTime().isValid())
            workout.setStartedTime(TimeProvider::instance().currentDateTime());
        if (!workout.endedTime().isValid())
            workout.setEndedTime(workout.startedTime().addSecs(3600));
        m_repository.save(workout);
    }
    return Result<void>::success();
}

Result<std::vector<WorkoutService::ExerciseFrequency>>
WorkoutService::topExercisesCore(int topN, int recentWorkouts)
{
    const std::vector<Workout> history = loadHistoryCore(recentWorkouts).value();

    QHash<QString, ExerciseFrequency> byName;

    for (const auto& workout : history)
    {
        for (const auto& exercise : workout.exercises())
        {
            if (!exercise.isWeighted())
                continue;

            ExerciseFrequency& entry = byName[exercise.name()];
            entry.name = exercise.name();
            entry.count += 1;
            entry.bestOneRepMax = std::max(entry.bestOneRepMax, exercise.bestOneRepMax());
        }
    }

    std::vector<ExerciseFrequency> result(byName.cbegin(), byName.cend());
    std::sort(result.begin(), result.end(),
              [](const ExerciseFrequency& a, const ExerciseFrequency& b)
              {
                  if (a.count != b.count)
                      return a.count > b.count;
                  if (a.bestOneRepMax != b.bestOneRepMax)
                      return a.bestOneRepMax > b.bestOneRepMax;
                  return a.name < b.name;
              });

    if (topN >= 0 && static_cast<int>(result.size()) > topN)
        result.resize(topN);

    return Result<std::vector<ExerciseFrequency>>::success(result);
}

Result<WorkoutService::TrainingTotals> WorkoutService::recentTotalsCore(int recentWorkouts)
{
    const std::vector<Workout> history = loadHistoryCore(recentWorkouts).value();

    TrainingTotals totals;
    totals.workouts = static_cast<int>(history.size());

    for (const auto& workout : history)
    {
        for (const auto& exercise : workout.exercises())
        {
            for (const auto& set : exercise.sets())
            {
                if (!set.completed())
                    continue;

                totals.totalWeight += set.totalWeight();
                totals.totalDurationSeconds += set.durationSeconds();
                totals.totalDistanceMeters += set.distanceMeters();
            }
        }
    }

    return Result<TrainingTotals>::success(totals);
}

Result<std::vector<WorkoutService::PreviousPerformance>>
WorkoutService::previousPerformancesCore(const Workout& workout)
{
    const std::vector<Workout> history = loadHistoryCore(previous_performance_window).value();

    std::vector<PreviousPerformance> result;
    const auto& exercises = workout.exercises();
    for (size_t i = 0; i < exercises.size(); ++i)
    {
        for (const Workout& past : history)
        {
            if (past.id() == workout.id())
                continue;

            const auto& candidates = past.exercises();
            const auto match = std::find_if(
                candidates.begin(), candidates.end(), [&](const Exercise& candidate)
                { return sameExercise(candidate, exercises[i]) && hasCompletedSet(candidate); });
            if (match == candidates.end())
                continue;

            result.push_back({ static_cast<int>(i), past.startedTime(), *match });
            break;
        }
    }

    return Result<std::vector<PreviousPerformance>>::success(result);
}

Result<std::optional<Workout>> WorkoutService::findWorkoutCore(int id)
{
    WorkoutQuery query;
    query.whereId(id);
    return Result<std::optional<Workout>>::success(m_repository.findOne(query));
}

Result<int> WorkoutService::saveWorkoutCore(const Workout& workout)
{
    return Result<int>::success(m_repository.save(workout));
}

Result<bool> WorkoutService::deleteWorkoutCore(int id)
{
    WorkoutQuery query;
    query.whereId(id);
    return Result<bool>::success(m_repository.remove(query));
}
