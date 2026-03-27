#pragma once

#include <QDateTime>
#include <QJsonArray>
#include <QList>
#include <QObject>
#include "ui/models/workoutmodel.h"
#include "core/storage/appdbstorage.h"
#include "utils/serializationutils.h"

class WorkoutHistoryViewModel : public QObject
{
    Q_OBJECT

    DECLARE_PROPERTY(QList<WorkoutModel *>, workouts, setWorkouts)
    Q_PROPERTY(WorkoutModel *lastWorkout READ lastWorkout NOTIFY lastWorkoutChanged)

public:
    explicit WorkoutHistoryViewModel(AppDbStorage *dbStorage, QObject *parent = nullptr);

    Q_INVOKABLE void loadAllWorkouts();
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
