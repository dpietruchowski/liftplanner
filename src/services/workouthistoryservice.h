#pragma once
#include <QDateTime>
#include <QJsonArray>
#include <QList>
#include <QObject>
#include "models/workoutmodel.h"
#include "storage/appdbstorage.h"

class WorkoutHistoryService : public QObject
{
    Q_OBJECT

    DECLARE_PROPERTY(QList<WorkoutModel *>, workouts, setWorkouts)
    Q_PROPERTY(WorkoutModel *lastWorkout READ lastWorkout NOTIFY lastWorkoutChanged)

public:
    explicit WorkoutHistoryService(AppDbStorage *dbStorage, QObject *parent = nullptr);

    Q_INVOKABLE void loadAllWorkouts();
    Q_INVOKABLE QList<WorkoutModel *> workoutsBetween(const QDateTime &from, const QDateTime &to);
    Q_INVOKABLE void saveWorkout(WorkoutModel *workout);
    Q_INVOKABLE void deleteWorkout(WorkoutModel *workout);

    Q_INVOKABLE QJsonArray recentWorkoutsToJson(int count = 10);

    WorkoutModel *lastWorkout() const;

signals:
    void errorOccurred(const QString &errorMessage);
    void lastWorkoutChanged();

private:
    AppDbStorage *m_dbStorage;
};
