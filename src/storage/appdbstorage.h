#pragma once
#include <QObject>
#include <QSqlDatabase>
#include "repositories/exercisedbrepository.h"
#include "repositories/setdbrepository.h"
#include "repositories/workoutdbrepository.h"

class AppDbStorage : public QObject
{
    Q_OBJECT

public:
    explicit AppDbStorage(const QString &dbPath, QObject *parent = nullptr);

    WorkoutDbRepository *workoutRepository();
    ExerciseDbRepository *exerciseRepository();
    SetDbRepository *setRepository();

    int saveWorkout(WorkoutModel *workout);
    void loadWorkout(WorkoutModel *workout);

private:
    void initializeDatabase();

    void loadExercisesForWorkout(WorkoutModel *workout);
    void loadSetsForExercise(ExerciseModel *exercise);

    QSqlDatabase m_database;
    WorkoutDbRepository *m_workoutRepo;
    ExerciseDbRepository *m_exerciseRepo;
    SetDbRepository *m_setRepo;
};
