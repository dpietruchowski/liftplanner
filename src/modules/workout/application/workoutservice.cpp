#include "workoutservice.h"
#include "modules/workout/domain/entities/workoutstatus.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/domain/repositories/workoutrepository.h"

WorkoutService::WorkoutService(WorkoutRepository &repository)
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

void WorkoutService::importPlannedWorkouts(const std::vector<Workout> &workouts)
{
    removeAllPlannedWorkouts();
    for (const auto &workout : workouts)
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

void WorkoutService::importHistory(const std::vector<Workout> &workouts)
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

std::optional<Workout> WorkoutService::findWorkout(int id) const
{
    WorkoutQuery query;
    query.whereId(id);
    return m_repository.findOne(query);
}

int WorkoutService::saveWorkout(const Workout &workout)
{
    return m_repository.save(workout);
}

bool WorkoutService::deleteWorkout(int id)
{
    WorkoutQuery query;
    query.whereId(id);
    return m_repository.remove(query);
}
