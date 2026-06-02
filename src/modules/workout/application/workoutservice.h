#pragma once

#include <QString>
#include <optional>
#include <vector>

#include "modules/workout/domain/entities/workout.h"

class WorkoutRepository;

class WorkoutService
{
public:
    struct ExerciseFrequency
    {
        QString name;
        int count { 0 };
        double bestOneRepMax { 0.0 };
    };

    explicit WorkoutService(WorkoutRepository& repository);

    // Planned workouts (not yet started, ordered by plannedTime)
    std::vector<Workout> loadPlannedWorkouts() const;
    void importPlannedWorkouts(const std::vector<Workout>& workouts);
    void removeAllPlannedWorkouts();

    // History (started/completed workouts)
    std::vector<Workout> loadHistory() const;
    void importHistory(const std::vector<Workout>& workouts);

    // Most frequently performed exercises across the most recent workouts,
    // each with its highest estimated one-rep max. Sorted by frequency
    // descending, then bestOneRepMax descending, then name.
    std::vector<ExerciseFrequency> topExercises(int topN, int recentWorkouts) const;

    // General CRUD
    std::optional<Workout> findWorkout(int id) const;
    int saveWorkout(const Workout& workout);
    bool deleteWorkout(int id);

private:
    WorkoutRepository& m_repository;
};
