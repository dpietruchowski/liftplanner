#include "exerciseserializer.h"
#include "modules/workout/domain/entities/exercise.h"

Exercise ExerciseSerializer::fromVariant(const QVariantMap& data)
{
    Exercise exercise;

    if (data.contains(id_key))
        exercise.setId(data.value(id_key).toInt());
    if (data.contains(workout_id_key))
        exercise.setWorkoutId(data.value(workout_id_key).toInt());
    if (data.contains(name_key))
        exercise.setName(data.value(name_key).toString());
    if (data.contains(description_key))
        exercise.setDescription(data.value(description_key).toString());
    if (data.contains(rest_seconds_key))
        exercise.setRestSeconds(data.value(rest_seconds_key).toInt());
    if (data.contains(kind_key))
        exercise.setKind(exerciseKindFromString(data.value(kind_key).toString()));
    if (data.contains(position_key))
        exercise.setPosition(data.value(position_key).toInt());
    if (data.contains(notes_key))
        exercise.setNotes(data.value(notes_key).toString());

    const QVariant definitionId = data.value(definition_id_key);
    if (definitionId.isValid() && !definitionId.isNull())
        exercise.setDefinitionId(definitionId.toInt());

    return exercise;
}

QVariantMap ExerciseSerializer::toVariant(const Exercise& exercise)
{
    QVariantMap data;

    if (exercise.id() != -1)
        data.insert(id_key, exercise.id());
    if (exercise.workoutId() != -1)
        data.insert(workout_id_key, exercise.workoutId());

    data.insert(name_key, exercise.name());
    data.insert(description_key, exercise.description());
    data.insert(rest_seconds_key, exercise.restSeconds());
    data.insert(kind_key, exerciseKindToString(exercise.kind()));
    data.insert(position_key, exercise.position());
    data.insert(notes_key, exercise.notes());
    data.insert(definition_id_key,
                exercise.hasDefinition() ? QVariant(exercise.definitionId().value()) : QVariant());

    return data;
}
