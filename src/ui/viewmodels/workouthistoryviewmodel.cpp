#include "workouthistoryviewmodel.h"

#include <QJsonArray>

WorkoutHistoryViewModel::WorkoutHistoryViewModel(AppDbStorage *dbStorage, QObject *parent)
    : QObject(parent), m_dbStorage(dbStorage)
{
    connect(this,
            &WorkoutHistoryViewModel::workoutsChanged,
            this,
            &WorkoutHistoryViewModel::lastWorkoutChanged);

    loadAllWorkouts();
}

void WorkoutHistoryViewModel::loadAllWorkouts()
{
    QString condition = "WHERE started_time IS NOT NULL ORDER BY started_time DESC";
    QVector<WorkoutModel *> workoutModels = m_dbStorage->loadAllWorkouts(condition);

    for (WorkoutModel *workout : workoutModels)
    {
        m_dbStorage->loadWorkout(workout);
    }

    auto workouts = workoutModels.toList();
    setWorkouts(workouts);
}

QList<WorkoutModel *> WorkoutHistoryViewModel::workoutsBetween(const QDateTime &from,
                                                               const QDateTime &to)
{
    QList<WorkoutModel *> result;
    for (WorkoutModel *w : m_workouts)
    {
        if (w->startedTime() >= from && w->endedTime() <= to)
            result.append(w);
    }
    return result;
}

void WorkoutHistoryViewModel::saveWorkout(WorkoutModel *workout)
{
    if (!m_dbStorage || !workout)
        return;
    if (!workout->startedTime().isValid())
        return;

    m_dbStorage->saveWorkout(workout);
    loadAllWorkouts();
}

void WorkoutHistoryViewModel::deleteWorkout(WorkoutModel *workout)
{
    if (!m_dbStorage || !workout)
        return;

    m_dbStorage->removeWorkouts(
        QString("WHERE id = %1").arg(workout->id()));

    loadAllWorkouts();
}

QJsonArray WorkoutHistoryViewModel::recentWorkoutsToJson(int count)
{
    QJsonArray jsonArray;

    int actualCount = qMin(count, m_workouts.size());
    for (int i = 0; i < actualCount; ++i)
    {
        WorkoutModel *workout = m_workouts.at(i);
        if (workout)
        {
            jsonArray.append(workout->toJson(SerializationMode::ChatGpt));
        }
    }

    return jsonArray;
}

WorkoutModel *WorkoutHistoryViewModel::lastWorkout() const
{
    if (m_workouts.isEmpty())
        return nullptr;

    return m_workouts.first();
}
