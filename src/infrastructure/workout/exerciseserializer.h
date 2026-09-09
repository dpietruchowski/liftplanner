#pragma once

#include <QVariantMap>

class Exercise;

class ExerciseSerializer
{
public:
    static constexpr const char* table = "exercises";
    static constexpr const char* id_key = "id";
    static constexpr const char* workout_id_key = "workout_id";
    static constexpr const char* name_key = "name";
    static constexpr const char* description_key = "description";
    static constexpr const char* rest_seconds_key = "rest_seconds";
    static constexpr const char* kind_key = "kind";
    static constexpr const char* position_key = "position";
    static constexpr const char* definition_id_key = "definition_id";
    static constexpr const char* notes_key = "notes";

    static Exercise fromVariant(const QVariantMap& data);
    static QVariantMap toVariant(const Exercise& exercise);
};
