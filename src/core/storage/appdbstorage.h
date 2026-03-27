#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <memory>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
#include "modules/workout/domain/repositories/workoutquery.h"

class DbStorage;
class WorkoutRepositoryDb;
class ExerciseRepositoryDb;
class SetRepositoryDb;

class AppDbStorage : public QObject
{
    Q_OBJECT

public:
    explicit AppDbStorage(const QString &dbPath, QObject *parent = nullptr);
    ~AppDbStorage() override;

    WorkoutRepositoryDb &workoutRepo();
    ExerciseRepositoryDb &exerciseRepo();
    SetRepositoryDb &setRepo();

    std::vector<Workout> loadWorkouts(const WorkoutQuery &query);
    Workout loadFullWorkout(int workoutId);
    int saveWorkoutEntity(const Workout &workout);
    bool removeWorkout(int workoutId);
    bool removeWorkoutsByQuery(const WorkoutQuery &query);

private:
    void initializeDatabase();

    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<ExerciseRepositoryDb> m_exerciseRepo;
    std::unique_ptr<SetRepositoryDb> m_setRepo;
};
