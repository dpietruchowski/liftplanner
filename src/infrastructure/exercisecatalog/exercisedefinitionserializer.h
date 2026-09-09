#pragma once

#include <QVariantMap>

class ExerciseDefinition;

class ExerciseDefinitionSerializer
{
public:
    static constexpr const char* table = "exercise_definitions";
    static constexpr const char* id_key = "id";
    static constexpr const char* slug_key = "slug";
    static constexpr const char* name_key = "name";
    static constexpr const char* aliases_key = "aliases";
    static constexpr const char* kind_key = "kind";
    static constexpr const char* default_load_type_key = "default_load_type";
    static constexpr const char* default_metric_key = "default_metric";
    static constexpr const char* equipment_key = "equipment";
    static constexpr const char* mechanics_key = "mechanics";
    static constexpr const char* laterality_key = "laterality";
    static constexpr const char* default_rest_seconds_key = "default_rest_seconds";
    static constexpr const char* video_url_key = "video_url";
    static constexpr const char* instructions_key = "instructions";
    static constexpr const char* origin_key = "origin";
    static constexpr const char* archived_key = "archived";

    static constexpr const char* alias_separator = "\n";

    static ExerciseDefinition fromVariant(const QVariantMap& data);
    static QVariantMap toVariant(const ExerciseDefinition& definition);
};
