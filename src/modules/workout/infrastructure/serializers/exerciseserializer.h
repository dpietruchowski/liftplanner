#pragma once

#include <QVariantMap>

class Exercise;

class ExerciseSerializer
{
public:
    static constexpr const char *table = "exercises";
    static constexpr const char *id_key = "id";
    static constexpr const char *workout_id_key = "workout_id";
    static constexpr const char *name_key = "name";
    static constexpr const char *description_key = "description";
    static constexpr const char *youtube_link_key = "youtube_link";
    static constexpr const char *rest_seconds_key = "rest_seconds";

    static Exercise fromVariant(const QVariantMap &data);
    static QVariantMap toVariant(const Exercise &exercise);
};
