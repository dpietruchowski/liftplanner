#pragma once

#include <QList>
#include <QObject>
#include "ui/models/workoutmodel.h"
#include "utils/serializationutils.h"

class WorkoutService;

class RoutineViewModel : public QObject
{
    Q_OBJECT
    DECLARE_PROPERTY(QString, lastWorkoutName, setLastWorkoutName)

    Q_PROPERTY(QList<WorkoutModel *> workouts READ workouts NOTIFY workoutsChanged)
    Q_PROPERTY(WorkoutModel *nextWorkout READ nextWorkout NOTIFY nextWorkoutChanged)

public:
    explicit RoutineViewModel(WorkoutService *service, QObject *parent = nullptr);
    ~RoutineViewModel();

    QList<WorkoutModel *> workouts() const;

    Q_INVOKABLE void loadAllWorkouts();
    Q_INVOKABLE void importWorkoutsFromJson(const QString &jsonData);
    Q_INVOKABLE void generateGptPrompt(const QJsonArray &recentTrainings);
    Q_INVOKABLE void importWorkoutsFromClipboard();

    WorkoutModel *nextWorkout() const;

private:
    WorkoutModel *next(const QString &name) const;

signals:
    void workoutsChanged();
    void nextWorkoutChanged();
    void errorOccurred(const QString &errorMessage);
    void gptPromptGenerated();

private:
    bool validateWorkoutsJson(const QString &jsonData, QString &errorMessage);

private:
    WorkoutService *m_service;
    QList<WorkoutModel *> m_workouts;
};
