#pragma once

#include "ui/models/workoutmodel.h"
#include "utils/serializationutils.h"
#include <QClipboard>
#include <QDateTime>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QObject>
#include <QVariantList>

class WorkoutService;
class ActiveWorkoutViewModel;

class WorkoutHistoryViewModel : public QObject
{
    Q_OBJECT

    DECLARE_PROPERTY(QList<WorkoutModel*>, workouts, setWorkouts)
    Q_PROPERTY(WorkoutModel* lastWorkout READ lastWorkout NOTIFY lastWorkoutChanged)
    Q_PROPERTY(QVariantList topExercises READ topExercises NOTIFY topExercisesChanged)
    Q_PROPERTY(QVariantList weekActivity READ weekActivity NOTIFY weekActivityChanged)

public:
    explicit WorkoutHistoryViewModel(WorkoutService* service,
                                     ActiveWorkoutViewModel* activeWorkoutViewModel = nullptr,
                                     QObject* parent = nullptr);

    Q_INVOKABLE void loadAllWorkouts();
    Q_INVOKABLE void saveWorkout(WorkoutModel* workout);
    Q_INVOKABLE void deleteWorkout(WorkoutModel* workout);
    Q_INVOKABLE void importFromJson(const QString& jsonData);
    Q_INVOKABLE void importFromClipboard();
    Q_INVOKABLE void exportToClipboard(int limit = 50);
    Q_INVOKABLE void exportWorkoutToClipboard(WorkoutModel* workout);

    Q_INVOKABLE QJsonArray recentWorkoutsToJson(int count = 10);

    WorkoutModel* lastWorkout() const;
    QVariantList topExercises() const;
    QVariantList weekActivity() const;

signals:
    void errorOccurred(const QString& errorMessage);
    void lastWorkoutChanged();
    void topExercisesChanged();
    void weekActivityChanged();
    void exportedToClipboard();

private:
    WorkoutService* m_service;
    ActiveWorkoutViewModel* m_activeWorkoutViewModel;
};
