#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <memory>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"

class DbStorage;
class WorkoutRepositoryDb;
class ExerciseRepositoryDb;
class SetRepositoryDb;
class WorkoutModel;
class ExerciseModel;
class SetModel;

class AppDbStorage : public QObject
{
    Q_OBJECT

public:
    explicit AppDbStorage(const QString &dbPath, QObject *parent = nullptr);
    ~AppDbStorage() override;

    WorkoutRepositoryDb &workoutRepo();
    ExerciseRepositoryDb &exerciseRepo();
    SetRepositoryDb &setRepo();

    int saveWorkout(WorkoutModel *workout);
    void loadWorkout(WorkoutModel *workout);

    static Workout workoutFromModel(const WorkoutModel *model);
    static Exercise exerciseFromModel(const ExerciseModel *model);
    static Set setFromModel(const SetModel *model);

    static void populateWorkoutModel(WorkoutModel *model, const Workout &entity);
    static void populateExerciseModel(ExerciseModel *model, const Exercise &entity);
    static void populateSetModel(SetModel *model, const Set &entity);

    QVector<WorkoutModel *> loadAllWorkouts(const QString &condition);
    bool removeWorkouts(const QString &condition);

private:
    void initializeDatabase();

    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<ExerciseRepositoryDb> m_exerciseRepo;
    std::unique_ptr<SetRepositoryDb> m_setRepo;
};
