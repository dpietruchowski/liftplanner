#pragma once

#include "ui/models/exercisemodel.h"
#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"
#include "utils/serializationutils.h"
#include "workouttimer.h"
#include <QObject>

class WorkoutService;

class ActiveWorkoutViewModel : public QObject
{
    Q_OBJECT
    DECLARE_PROPERTY(WorkoutModel*, currentWorkout, setCurrentWorkout)
    DECLARE_PROPERTY(ExerciseModel*, currentExercise, setCurrentExercise)
    DECLARE_PROPERTY(SetModel*, currentSet, setCurrentSet)
    DECLARE_PROPERTY(bool, isActive, setIsActive)
    Q_PROPERTY(WorkoutTimer* timer READ timer CONSTANT)

public:
    explicit ActiveWorkoutViewModel(WorkoutService* service, QObject* parent = nullptr);
    ~ActiveWorkoutViewModel();

    WorkoutTimer* timer() const;

    Q_INVOKABLE void saveCurrentWorkout();
    void loadCurrentWorkout();

    Q_INVOKABLE void startWorkout(WorkoutModel* workout);
    Q_INVOKABLE void completeCurrentSet();
    Q_INVOKABLE void navigateToNext();
    Q_INVOKABLE void navigateToPrevious();
    Q_INVOKABLE void endWorkout();

    Q_INVOKABLE void duplicateSet(SetModel* set);
    Q_INVOKABLE void removeSet(SetModel* set);
    Q_INVOKABLE void toggleSetCompleted(SetModel* set);
    Q_INVOKABLE void adjustSetPrimary(SetModel* set, int steps);
    Q_INVOKABLE void adjustSetSecondary(SetModel* set, int steps);
    Q_INVOKABLE void moveExercise(int from, int to);

    Q_INVOKABLE void startWorkTimer();
    Q_INVOKABLE void startRestTimer();
    Q_INVOKABLE void toggleTimer();

signals:
    void workoutCompleted();
    void errorOccurred(const QString& errorMessage);

private:
    void onTimerFinished(WorkoutTimer::Phase phase);
    void startRestAfterCompletedSet(int restSeconds);
    void startWorkForCurrentSet();
    int restSecondsFor(SetModel* set) const;
    int workSecondsFor(SetModel* set) const;

    void saveCompletedSet();
    void updateCurrentExercise();
    void updateCurrentSet();
    void selectFirstIncomplete();
    void selectNextIncomplete();
    void saveToDb();

    WorkoutService* m_service;
    WorkoutTimer* m_timer;
};
