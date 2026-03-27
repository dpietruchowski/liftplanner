#include "workoutservice.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/domain/repositories/workoutrepository.h"

WorkoutService::WorkoutService(WorkoutRepository &repository)
    : m_repository(repository)
{
}

std::vector<Workout> WorkoutService::loadPlannedWorkouts() const
{
    WorkoutQuery query;
    query.whereStartedTimeIsNull();
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
    query.whereStartedTimeIsNull();
    m_repository.remove(query);
}

std::vector<Workout> WorkoutService::loadHistory() const
{
    WorkoutQuery query;
    query.whereStartedTimeIsNotNull();
    query.orderByStartedTime(SortDirection::Descending);
    return m_repository.findAll(query);
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
