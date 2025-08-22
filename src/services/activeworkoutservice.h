#pragma once
#include <QObject>
#include "../models/workoutmodel.h"
#include "../models/exercisemodel.h"
#include "../models/setmodel.h"
#include "utils/serializationutils.h"

class ActiveWorkoutService : public QObject
{
    Q_OBJECT
    DECLARE_PROPERTY(WorkoutModel *, currentWorkout, setCurrentWorkout)
    DECLARE_PROPERTY(ExerciseModel *, currentExercise, setCurrentExercise)
    DECLARE_PROPERTY(SetModel *, currentSet, setCurrentSet)
    DECLARE_PROPERTY(bool, isActive, setIsActive)

public:
    explicit ActiveWorkoutService(QObject *parent = nullptr);

    Q_INVOKABLE void startWorkout(WorkoutModel *workout);
    Q_INVOKABLE void completeCurrentSet();
    Q_INVOKABLE void navigateToNext();
    Q_INVOKABLE void navigateToPrevious();
    Q_INVOKABLE void endWorkout();

signals:
    void workoutCompleted();
    void errorOccurred(const QString &errorMessage);

private:
    void saveCompletedSet();
    void updateCurrentExercise();
    void updateCurrentSet();
};
