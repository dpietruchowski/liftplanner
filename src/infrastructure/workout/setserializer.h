#pragma once

#include <QVariantMap>

class Set;

class SetSerializer
{
public:
    static constexpr const char* table = "sets";
    static constexpr const char* id_key = "id";
    static constexpr const char* exercise_id_key = "exercise_id";
    static constexpr const char* repetitions_key = "repetitions";
    static constexpr const char* weight_key = "weight";
    static constexpr const char* completed_key = "completed";
    static constexpr const char* metric_key = "metric";
    static constexpr const char* load_type_key = "load_type";
    static constexpr const char* duration_seconds_key = "duration_seconds";
    static constexpr const char* distance_meters_key = "distance_meters";
    static constexpr const char* rest_seconds_override_key = "rest_seconds_override";
    static constexpr const char* position_key = "position";

    static Set fromVariant(const QVariantMap& data);
    static QVariantMap toVariant(const Set& set);
};
