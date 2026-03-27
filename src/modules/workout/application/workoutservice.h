#pragma once

#include <optional>
#include <vector>

#include "modules/workout/domain/entities/workout.h"

class WorkoutRepository;

class WorkoutService
{
public:
    explicit WorkoutService(WorkoutRepository &repository);

    // Planned workouts (not yet started, ordered by plannedTime)
    std::vector<Workout> loadPlannedWorkouts() const;
    void importPlannedWorkouts(const std::vector<Workout> &workouts);
    void removeAllPlannedWorkouts();

    // History (started/completed workouts)
    std::vector<Workout> loadHistory() const;

    // General CRUD
    std::optional<Workout> findWorkout(int id) const;
    int saveWorkout(const Workout &workout);
    bool deleteWorkout(int id);

private:
    WorkoutRepository &m_repository;
};
