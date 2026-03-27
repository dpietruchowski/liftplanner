#include "workoutserializer.h"
#include "modules/workout/domain/entities/workout.h"

#include <QDateTime>

Workout WorkoutSerializer::fromVariant(const QVariantMap &data)
{
    Workout workout;

    if (data.contains(id_key))
        workout.setId(data.value(id_key).toInt());
    if (data.contains(name_key))
        workout.setName(data.value(name_key).toString());
    if (data.contains(created_time_key))
        workout.setCreatedTime(QDateTime::fromString(data.value(created_time_key).toString(), Qt::ISODate));
    if (data.contains(started_time_key))
        workout.setStartedTime(QDateTime::fromString(data.value(started_time_key).toString(), Qt::ISODate));
    if (data.contains(ended_time_key))
        workout.setEndedTime(QDateTime::fromString(data.value(ended_time_key).toString(), Qt::ISODate));

    return workout;
}

QVariantMap WorkoutSerializer::toVariant(const Workout &workout)
{
    QVariantMap data;

    if (workout.id() != -1)
        data.insert(id_key, workout.id());

    data.insert(name_key, workout.name());

    if (workout.createdTime().isValid())
        data.insert(created_time_key, workout.createdTime().toString(Qt::ISODate));
    if (workout.startedTime().isValid())
        data.insert(started_time_key, workout.startedTime().toString(Qt::ISODate));
    if (workout.endedTime().isValid())
        data.insert(ended_time_key, workout.endedTime().toString(Qt::ISODate));

    return data;
}
