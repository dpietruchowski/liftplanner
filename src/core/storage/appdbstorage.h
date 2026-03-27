#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <memory>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/repositories/workoutquery.h"

class DbStorage;
class WorkoutRepositoryDb;

class AppDbStorage : public QObject
{
    Q_OBJECT

public:
    explicit AppDbStorage(const QString &dbPath, QObject *parent = nullptr);
    ~AppDbStorage() override;

    WorkoutRepositoryDb &workoutRepo();

    std::vector<Workout> loadWorkouts(const WorkoutQuery &query);
    Workout loadFullWorkout(int workoutId);
    int saveWorkout(const Workout &workout);
    bool removeWorkout(int workoutId);
    bool removeWorkoutsByQuery(const WorkoutQuery &query);

private:
    void initializeDatabase();

    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
};
