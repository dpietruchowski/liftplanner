#include "setserializer.h"
#include "modules/workout/domain/entities/set.h"

Set SetSerializer::fromVariant(const QVariantMap& data)
{
    Set set;

    if (data.contains(id_key))
        set.setId(data.value(id_key).toInt());
    if (data.contains(exercise_id_key))
        set.setExerciseId(data.value(exercise_id_key).toInt());
    if (data.contains(repetitions_key))
        set.setRepetitions(data.value(repetitions_key).toInt());
    if (data.contains(weight_key))
        set.setWeight(data.value(weight_key).toDouble());
    if (data.contains(completed_key))
        set.setCompleted(data.value(completed_key).toBool());
    if (data.contains(metric_key))
        set.setMetric(setMetricFromString(data.value(metric_key).toString()));
    if (data.contains(load_type_key))
        set.setLoadType(loadTypeFromString(data.value(load_type_key).toString()));
    if (data.contains(duration_seconds_key))
        set.setDurationSeconds(data.value(duration_seconds_key).toInt());
    if (data.contains(distance_meters_key))
        set.setDistanceMeters(data.value(distance_meters_key).toDouble());
    if (data.contains(rest_seconds_override_key))
        set.setRestSecondsOverride(data.value(rest_seconds_override_key).toInt());

    return set;
}

QVariantMap SetSerializer::toVariant(const Set& set)
{
    QVariantMap data;

    if (set.id() != -1)
        data.insert(id_key, set.id());
    if (set.exerciseId() != -1)
        data.insert(exercise_id_key, set.exerciseId());

    data.insert(repetitions_key, set.repetitions());
    data.insert(weight_key, set.weight());
    data.insert(completed_key, set.completed());
    data.insert(metric_key, setMetricToString(set.metric()));
    data.insert(load_type_key, loadTypeToString(set.loadType()));
    data.insert(duration_seconds_key, set.durationSeconds());
    data.insert(distance_meters_key, set.distanceMeters());
    data.insert(rest_seconds_override_key, set.restSecondsOverride());

    return data;
}
