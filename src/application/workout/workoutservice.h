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

    struct ExerciseSession
    {
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
    Task<std::optional<Exercise>> lastPerformance(const Exercise& exercise);
    Task<std::vector<ExerciseSession>> exerciseSessions(const Exercise& exercise, int limit);

    Task<std::optional<Workout>> findWorkout(int id);
    Task<int> repeatWorkout(int id, const QDateTime& plannedTime);
    Task<int> saveWorkout(const Workout& workout);
    Task<bool> saveSet(const Set& set);
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
    Result<std::optional<Exercise>> lastPerformanceCore(const Exercise& exercise);
    Result<std::vector<ExerciseSession>> exerciseSessionsCore(const Exercise& exercise, int limit);
    Result<std::optional<Workout>> findWorkoutCore(int id);
    Result<int> repeatWorkoutCore(int id, const QDateTime& plannedTime);
    Result<int> saveWorkoutCore(const Workout& workout);
    Result<bool> saveSetCore(const Set& set);
    Result<bool> deleteWorkoutCore(int id);

    WorkoutRepository& m_repository;
    WorkoutRowRepository& m_rowRepository;
};
