#include "setserializer.h"
#include "modules/workout/domain/entities/set.h"

Set SetSerializer::fromVariant(const QVariantMap &data)
{
    Set set;

    if (data.contains(id_key))
        set.setId(data.value(id_key).toInt());
    if (data.contains(exercise_id_key))
        set.setExerciseId(data.value(exercise_id_key).toInt());
    if (data.contains(repetitions_key))
        set.setRepetitions(data.value(repetitions_key).toInt());
    if (data.contains(weight_key))
        set.setWeight(data.value(weight_key).toInt());

    return set;
}

QVariantMap SetSerializer::toVariant(const Set &set)
{
    QVariantMap data;

    if (set.id() != -1)
        data.insert(id_key, set.id());
    if (set.exerciseId() != -1)
        data.insert(exercise_id_key, set.exerciseId());

    data.insert(repetitions_key, set.repetitions());
    data.insert(weight_key, set.weight());

    return data;
}
