#pragma once
#include <QList>
#include <QObject>
#include "models/workoutmodel.h"
#include "storage/appdbstorage.h"

class RoutineService : public QObject
{
    Q_OBJECT
    DECLARE_MODEL_PROPERTY(QString, lastWorkoutName, setLastWorkoutName, lastWorkoutName)

    Q_PROPERTY(QList<WorkoutModel *> workouts READ workouts NOTIFY workoutsChanged)
    Q_PROPERTY(WorkoutModel *nextWorkout READ nextWorkout NOTIFY nextWorkoutChanged)

public:
    explicit RoutineService(AppDbStorage *dbStorage, QObject *parent = nullptr);
    ~RoutineService();

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
    AppDbStorage *m_dbStorage;
    QList<WorkoutModel *> m_workouts;
};
