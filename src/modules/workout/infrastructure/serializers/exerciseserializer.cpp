#include "exerciseserializer.h"
#include "modules/workout/domain/entities/exercise.h"

Exercise ExerciseSerializer::fromVariant(const QVariantMap &data)
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
    if (data.contains(youtube_link_key))
        exercise.setYoutubeLink(data.value(youtube_link_key).toString());
    if (data.contains(rest_seconds_key))
        exercise.setRestSeconds(data.value(rest_seconds_key).toInt());

    return exercise;
}

QVariantMap ExerciseSerializer::toVariant(const Exercise &exercise)
{
    QVariantMap data;

    if (exercise.id() != -1)
        data.insert(id_key, exercise.id());
    if (exercise.workoutId() != -1)
        data.insert(workout_id_key, exercise.workoutId());

    data.insert(name_key, exercise.name());
    data.insert(description_key, exercise.description());
    data.insert(youtube_link_key, exercise.youtubeLink());
    data.insert(rest_seconds_key, exercise.restSeconds());

    return data;
}
