#include "workouthistoryservice.h"

WorkoutHistoryService::WorkoutHistoryService(AppDbStorage *dbStorage, QObject *parent)
    : QObject(parent), m_dbStorage(dbStorage)
{
    loadAllWorkouts();
}

void WorkoutHistoryService::loadAllWorkouts()
{
    QString condition = "WHERE started_time IS NOT NULL";
    QVector<WorkoutModel *> workoutModels = m_dbStorage->workoutRepository()->loadAll(condition);
    for (WorkoutModel *workout : workoutModels) {
        m_dbStorage->loadWorkout(workout);
        m_workouts.append(workout);
    }
    setWorkouts(workoutModels);
}

QList<WorkoutModel *> WorkoutHistoryService::workoutsBetween(const QDateTime &from, const QDateTime &to)
{
    QList<WorkoutModel *> result;
    for (WorkoutModel *w : m_workouts)
    {
        if (w->startedTime() >= from && w->endedTime() <= to)
            result.append(w);
    }
    return result;
}

void WorkoutHistoryService::saveWorkout(WorkoutModel *workout)
{
    if (!m_dbStorage || !workout)
        return;
    if (!workout->startedTime().isValid())
        return;

    m_dbStorage->saveWorkout(workout);
    loadAllWorkouts();
}

void WorkoutHistoryService::deleteWorkout(WorkoutModel *workout)
{
    if (!m_dbStorage || !workout)
        return;

    m_dbStorage->workoutRepository()->remove(
        QString("WHERE id = %1").arg(workout->id()));

    loadAllWorkouts();
}
