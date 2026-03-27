#include "workouthistoryviewmodel.h"
#include "utils/workoutjson.h"
#include "modules/workout/domain/repositories/workoutquery.h"

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
    qDeleteAll(m_workouts);
    m_workouts.clear();

    WorkoutQuery query;
    query.whereStartedTimeIsNotNull();
    query.orderByStartedTime(SortDirection::Descending);

    auto entities = m_dbStorage->loadWorkouts(query);
    for (const auto &entity : entities)
        m_workouts.append(new WorkoutModel(entity, this));

    emit workoutsChanged();
}

void WorkoutHistoryViewModel::saveWorkout(WorkoutModel *workout)
{
    if (!m_dbStorage || !workout)
        return;
    if (!workout->startedTime().isValid())
        return;

    Workout entity = workout->toEntity();
    m_dbStorage->saveWorkout(entity);
    loadAllWorkouts();
}

void WorkoutHistoryViewModel::deleteWorkout(WorkoutModel *workout)
{
    if (!m_dbStorage || !workout)
        return;

    m_dbStorage->removeWorkout(workout->id());
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
            jsonArray.append(WorkoutJson::workoutToJsonCompact(workout->toEntity()));
    }

    return jsonArray;
}

WorkoutModel *WorkoutHistoryViewModel::lastWorkout() const
{
    if (m_workouts.isEmpty())
        return nullptr;

    return m_workouts.first();
}
