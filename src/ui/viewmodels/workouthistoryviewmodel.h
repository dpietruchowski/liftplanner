#pragma once

#include <QDateTime>
#include <QJsonArray>
#include <QList>
#include <QObject>
#include "ui/models/workoutmodel.h"
#include "utils/serializationutils.h"

class WorkoutService;

class WorkoutHistoryViewModel : public QObject
{
    Q_OBJECT

    DECLARE_PROPERTY(QList<WorkoutModel *>, workouts, setWorkouts)
    Q_PROPERTY(WorkoutModel *lastWorkout READ lastWorkout NOTIFY lastWorkoutChanged)

public:
    explicit WorkoutHistoryViewModel(WorkoutService *service, QObject *parent = nullptr);

    Q_INVOKABLE void loadAllWorkouts();
    Q_INVOKABLE void saveWorkout(WorkoutModel *workout);
    Q_INVOKABLE void deleteWorkout(WorkoutModel *workout);

    Q_INVOKABLE QJsonArray recentWorkoutsToJson(int count = 10);

    WorkoutModel *lastWorkout() const;

signals:
    void errorOccurred(const QString &errorMessage);
    void lastWorkoutChanged();

private:
    WorkoutService *m_service;
};
