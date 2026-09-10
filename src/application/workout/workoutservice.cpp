#include "workoutservice.h"
#include "async/timeprovider.h"
#include "domain/workout/historysetrow.h"
#include "domain/workout/performedsets.h"
#include "domain/workout/plannedworkout.h"
#include "domain/workout/strengthmath.h"
#include "domain/workout/workoutquery.h"
#include "domain/workout/workoutrepeat.h"
#include "domain/workout/workoutrepository.h"
#include "domain/workout/workoutrowrepository.h"
#include "domain/workout/workoutstatus.h"
#include <QHash>
#include <QSet>
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

}

WorkoutService::WorkoutService(WorkoutRepository& repository, WorkoutRowRepository& rowRepository,
                               QObject* worker)
    : Service(worker)
    , m_repository(repository)
    , m_rowRepository(rowRepository)
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

Task<std::optional<Exercise>> WorkoutService::lastPerformance(const Exercise& exercise)
{
    return invoke([this, exercise] { return lastPerformanceCore(exercise); });
}

Task<std::optional<Workout>> WorkoutService::findWorkout(int id)
{
    return invoke([this, id] { return findWorkoutCore(id); });
}

Task<int> WorkoutService::repeatWorkout(int id, const QDateTime& plannedTime)
{
    return invoke([this, id, plannedTime] { return repeatWorkoutCore(id, plannedTime); });
}

Task<int> WorkoutService::saveWorkout(const Workout& workout)
{
    return invoke([this, workout] { return saveWorkoutCore(workout); });
}

Task<bool> WorkoutService::saveSet(const Set& set)
{
    return invoke([this, set] { return saveSetCore(set); });
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
        m_repository.save(asPlanned(workout));
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
    QHash<QString, ExerciseFrequency> byName;

    const auto foldExercise = [&byName](const QString& name, bool weighted, double bestOneRepMax)
    {
        if (!weighted)
            return;

        ExerciseFrequency& entry = byName[name];
        entry.name = name;
        entry.count += 1;
        entry.bestOneRepMax = std::max(entry.bestOneRepMax, bestOneRepMax);
    };

    const std::vector<HistorySetRow> rows
        = m_rowRepository.findSetsOfRecentWorkouts(recentWorkouts);
    const QSet<int> meaningfulFlags = sessionsWithMeaningfulFlags(rows);

    int currentWorkoutId = -1;
    int currentExerciseId = -1;
    QString currentName;
    bool currentWeighted = false;
    double currentBest = 0.0;

    for (const HistorySetRow& row : rows)
    {
        if (row.exerciseId != currentExerciseId || row.workoutId != currentWorkoutId)
        {
            foldExercise(currentName, currentWeighted, currentBest);
            currentWorkoutId = row.workoutId;
            currentExerciseId = row.exerciseId;
            currentName = row.exerciseName;
            currentWeighted = false;
            currentBest = 0.0;
        }

        if (!wasPerformed(row, meaningfulFlags.contains(row.workoutId))
            || !StrengthMath::isWeighted(row.metric, row.loadType))
            continue;

        currentWeighted = true;
        currentBest = std::max(currentBest, StrengthMath::oneRepMax(row.repetitions, row.weight));
    }
    foldExercise(currentName, currentWeighted, currentBest);

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
    TrainingTotals totals;
    QSet<int> workoutIds;

    const std::vector<HistorySetRow> rows
        = m_rowRepository.findSetsOfRecentWorkouts(recentWorkouts);
    const QSet<int> meaningfulFlags = sessionsWithMeaningfulFlags(rows);

    for (const HistorySetRow& row : rows)
    {
        workoutIds.insert(row.workoutId);

        if (!wasPerformed(row, meaningfulFlags.contains(row.workoutId)))
            continue;

        if (StrengthMath::isWeighted(row.metric, row.loadType))
            totals.totalWeight += StrengthMath::volume(row.repetitions, row.weight);

        totals.totalDurationSeconds += row.durationSeconds;
        totals.totalDistanceMeters += row.distanceMeters;
    }

    totals.workouts = workoutIds.size();

    return Result<TrainingTotals>::success(totals);
}

Result<std::vector<WorkoutService::PreviousPerformance>>
WorkoutService::previousPerformancesCore(const Workout& workout)
{
    const std::vector<Workout> history = loadHistoryCore(previous_performance_window).value();

    std::vector<bool> flagsAreMeaningful;
    flagsAreMeaningful.reserve(history.size());
    for (const Workout& past : history)
        flagsAreMeaningful.push_back(completionFlagsAreMeaningful(past));

    std::vector<PreviousPerformance> result;
    const auto& exercises = workout.exercises();
    for (size_t i = 0; i < exercises.size(); ++i)
    {
        for (size_t h = 0; h < history.size(); ++h)
        {
            const Workout& past = history[h];
            if (past.id() == workout.id())
                continue;

            const bool trustFlags = flagsAreMeaningful[h];
            const auto& candidates = past.exercises();
            const auto match = std::find_if(candidates.begin(), candidates.end(),
                                            [&](const Exercise& candidate) {
                                                return sameExercise(candidate, exercises[i])
                                                    && wasPerformed(candidate, trustFlags);
                                            });
            if (match == candidates.end())
                continue;

            result.push_back(
                { static_cast<int>(i), past.startedTime(), asPerformed(*match, trustFlags) });
            break;
        }
    }

    return Result<std::vector<PreviousPerformance>>::success(result);
}

Result<std::optional<Exercise>> WorkoutService::lastPerformanceCore(const Exercise& exercise)
{
    Workout probe;
    probe.addExercise(exercise);

    const std::vector<PreviousPerformance> found = previousPerformancesCore(probe).value();
    if (found.empty())
        return Result<std::optional<Exercise>>::success(std::nullopt);

    return Result<std::optional<Exercise>>::success(found.front().exercise);
}

Result<std::optional<Workout>> WorkoutService::findWorkoutCore(int id)
{
    WorkoutQuery query;
    query.whereId(id);
    return Result<std::optional<Workout>>::success(m_repository.findOne(query));
}

Result<int> WorkoutService::repeatWorkoutCore(int id, const QDateTime& plannedTime)
{
    const std::optional<Workout> source = findWorkoutCore(id).value();
    if (!source.has_value())
        return Result<int>::failure(QStringLiteral("workout %1 does not exist").arg(id));

    return saveWorkoutCore(repeatOf(source.value(), plannedTime));
}

Result<int> WorkoutService::saveWorkoutCore(const Workout& workout)
{
    return Result<int>::success(m_repository.save(workout));
}

Result<bool> WorkoutService::saveSetCore(const Set& set)
{
    return Result<bool>::success(m_repository.saveSet(set));
}

Result<bool> WorkoutService::deleteWorkoutCore(int id)
{
    WorkoutQuery query;
    query.whereId(id);
    return Result<bool>::success(m_repository.remove(query));
}
