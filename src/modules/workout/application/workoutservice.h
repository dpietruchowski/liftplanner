#pragma once

#include <optional>
#include <vector>

#include "modules/workout/domain/entities/workout.h"

class WorkoutRepository;

class WorkoutService
{
public:
    explicit WorkoutService(WorkoutRepository &repository);

    // Routines (workout templates, not yet started)
    std::vector<Workout> loadRoutines() const;
    void importRoutines(const std::vector<Workout> &workouts);
    void removeAllRoutines();

    // History (started/completed workouts)
    std::vector<Workout> loadHistory() const;

    // General CRUD
    std::optional<Workout> findWorkout(int id) const;
    int saveWorkout(const Workout &workout);
    bool deleteWorkout(int id);

private:
    WorkoutRepository &m_repository;
};
