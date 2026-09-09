#pragma once

#include <QDateTime>
#include <QString>
#include <optional>
#include <vector>

#include "async/service.h"
#include "async/testing.h"
#include "domain/workout/exercise.h"
#include "domain/workout/workout.h"

class WorkoutRepository;
class WorkoutRowRepository;

class WorkoutService final : public Service
{
public:
    struct ExerciseFrequency
    {
        QString name;
        int count { 0 };
        double bestOneRepMax { 0.0 };
    };

    struct TrainingTotals
    {
        int workouts { 0 };
        double totalWeight { 0.0 };
        int totalDurationSeconds { 0 };
        double totalDistanceMeters { 0.0 };
    };

    struct PreviousPerformance
    {
        int exerciseIndex { -1 };
        QDateTime performedAt;
        Exercise exercise;
    };

    WorkoutService(WorkoutRepository& repository, WorkoutRowRepository& rowRepository,
                   QObject* worker);

    Task<std::vector<Workout>> loadPlannedWorkouts();
    Task<void> importPlannedWorkouts(const std::vector<Workout>& workouts);
    Task<void> removeAllPlannedWorkouts();

    Task<std::vector<Workout>> loadHistory(int limit = -1);
    Task<void> importHistory(const std::vector<Workout>& workouts);

    Task<std::vector<ExerciseFrequency>> topExercises(int topN, int recentWorkouts);
    Task<TrainingTotals> recentTotals(int recentWorkouts);
    Task<std::vector<PreviousPerformance>> previousPerformances(const Workout& workout);

    Task<std::optional<Workout>> findWorkout(int id);
    Task<int> saveWorkout(const Workout& workout);
    Task<bool> deleteWorkout(int id);

private:
    LIBS_TEST_FRIEND(WorkoutServiceTest)

    Result<std::vector<Workout>> loadPlannedWorkoutsCore();
    Result<void> importPlannedWorkoutsCore(const std::vector<Workout>& workouts);
    Result<void> removeAllPlannedWorkoutsCore();
    Result<std::vector<Workout>> loadHistoryCore(int limit);
    Result<void> importHistoryCore(const std::vector<Workout>& workouts);
    Result<std::vector<ExerciseFrequency>> topExercisesCore(int topN, int recentWorkouts);
    Result<TrainingTotals> recentTotalsCore(int recentWorkouts);
    Result<std::vector<PreviousPerformance>> previousPerformancesCore(const Workout& workout);
    Result<std::optional<Workout>> findWorkoutCore(int id);
    Result<int> saveWorkoutCore(const Workout& workout);
    Result<bool> deleteWorkoutCore(int id);

    WorkoutRepository& m_repository;
    WorkoutRowRepository& m_rowRepository;
};
