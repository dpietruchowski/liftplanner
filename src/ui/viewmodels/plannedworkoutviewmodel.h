#pragma once

#include <QList>
#include <QObject>
#include "ui/models/workoutmodel.h"

class WorkoutService;

class PlannedWorkoutViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<WorkoutModel *> workouts READ workouts NOTIFY workoutsChanged)
    Q_PROPERTY(WorkoutModel *nextWorkout READ nextWorkout NOTIFY workoutsChanged)

public:
    explicit PlannedWorkoutViewModel(WorkoutService *service, QObject *parent = nullptr);
    ~PlannedWorkoutViewModel();

    QList<WorkoutModel *> workouts() const;
    WorkoutModel *nextWorkout() const;

    Q_INVOKABLE void loadAll();
    Q_INVOKABLE void importFromClipboard();
    Q_INVOKABLE void importFromJson(const QString &jsonData);
    Q_INVOKABLE void generatePrompt();

signals:
    void workoutsChanged();
    void errorOccurred(const QString &errorMessage);
    void promptGenerated();

private:
    bool validateJson(const QString &jsonData, QString &errorMessage);
    static QString readTemplateFile(const QString &filePath);

    WorkoutService *m_service;
    QList<WorkoutModel *> m_workouts;
};
