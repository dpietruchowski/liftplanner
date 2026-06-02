#include "workoutservice.h"
#include "modules/workout/domain/entities/workoutstatus.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/domain/repositories/workoutrepository.h"
#include <QHash>
#include <algorithm>

WorkoutService::WorkoutService(WorkoutRepository& repository)
    : m_repository(repository)
{
}

std::vector<Workout> WorkoutService::loadPlannedWorkouts() const
{
    WorkoutQuery query;
    query.whereStatus(WorkoutStatus::Planned);
    query.orderByPlannedTime(SortDirection::Ascending);
    return m_repository.findAll(query);
}

void WorkoutService::importPlannedWorkouts(const std::vector<Workout>& workouts)
{
    removeAllPlannedWorkouts();
    for (const auto& workout : workouts)
        m_repository.save(workout);
}

void WorkoutService::removeAllPlannedWorkouts()
{
    WorkoutQuery query;
    query.whereStatus(WorkoutStatus::Planned);
    m_repository.remove(query);
}

std::vector<Workout> WorkoutService::loadHistory() const
{
    WorkoutQuery query;
    query.whereStatus(WorkoutStatus::Ended);
    query.orderByStartedTime(SortDirection::Descending);
    return m_repository.findAll(query);
}

void WorkoutService::importHistory(const std::vector<Workout>& workouts)
{
    for (auto workout : workouts)
    {
        workout.setStatus(WorkoutStatus::Ended);
        if (!workout.startedTime().isValid())
            workout.setStartedTime(QDateTime::currentDateTime());
        if (!workout.endedTime().isValid())
            workout.setEndedTime(workout.startedTime().addSecs(3600));
        m_repository.save(workout);
    }
}

std::vector<WorkoutService::ExerciseFrequency> WorkoutService::topExercises(int topN,
                                                                           int recentWorkouts) const
{
    const std::vector<Workout> history = loadHistory();

    QHash<QString, ExerciseFrequency> byName;

    const int limit = std::min(recentWorkouts, static_cast<int>(history.size()));
    for (int i = 0; i < limit; ++i)
    {
        for (const auto& exercise : history[i].exercises())
        {
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

    return result;
}

std::optional<Workout> WorkoutService::findWorkout(int id) const
{
    WorkoutQuery query;
    query.whereId(id);
    return m_repository.findOne(query);
}

int WorkoutService::saveWorkout(const Workout& workout) { return m_repository.save(workout); }

bool WorkoutService::deleteWorkout(int id)
{
    WorkoutQuery query;
    query.whereId(id);
    return m_repository.remove(query);
}
