#pragma once

#include <QObject>
#include "ui/models/workoutmodel.h"
#include "ui/models/exercisemodel.h"
#include "ui/models/setmodel.h"
#include "utils/serializationutils.h"

class WorkoutService;

class ActiveWorkoutViewModel : public QObject
{
    Q_OBJECT
    DECLARE_PROPERTY(WorkoutModel *, currentWorkout, setCurrentWorkout)
    DECLARE_PROPERTY(ExerciseModel *, currentExercise, setCurrentExercise)
    DECLARE_PROPERTY(SetModel *, currentSet, setCurrentSet)
    DECLARE_PROPERTY(bool, isActive, setIsActive)

public:
    explicit ActiveWorkoutViewModel(WorkoutService *service, QObject *parent = nullptr);
    ~ActiveWorkoutViewModel();

    void saveCurrentWorkout();
    void loadCurrentWorkout();

    Q_INVOKABLE void startWorkout(WorkoutModel *workout);
    Q_INVOKABLE void completeCurrentSet();
    Q_INVOKABLE void navigateToNext();
    Q_INVOKABLE void navigateToPrevious();
    Q_INVOKABLE void endWorkout();

    Q_INVOKABLE void duplicateSet(SetModel *set);
    Q_INVOKABLE void removeSet(SetModel *set);

signals:
    void workoutCompleted();
    void errorOccurred(const QString &errorMessage);

private:
    void saveCompletedSet();
    void updateCurrentExercise();
    void updateCurrentSet();
    void saveToDb();

    WorkoutService *m_service;
};
