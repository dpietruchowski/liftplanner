#include "workouthistoryviewmodel.h"
#include "modules/workout/application/workoutservice.h"
#include "utils/workoutjson.h"

WorkoutHistoryViewModel::WorkoutHistoryViewModel(WorkoutService *service, QObject *parent)
    : QObject(parent), m_service(service)
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

    auto entities = m_service->loadHistory();
    for (const auto &entity : entities)
        m_workouts.append(new WorkoutModel(entity, this));

    emit workoutsChanged();
}

void WorkoutHistoryViewModel::saveWorkout(WorkoutModel *workout)
{
    if (!m_service || !workout)
        return;
    if (!workout->startedTime().isValid())
        return;

    Workout entity = workout->toEntity();
    m_service->saveWorkout(entity);
    loadAllWorkouts();
}

void WorkoutHistoryViewModel::deleteWorkout(WorkoutModel *workout)
{
    if (!m_service || !workout)
        return;

    m_service->deleteWorkout(workout->id());
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
