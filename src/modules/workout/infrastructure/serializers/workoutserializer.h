#pragma once

#include <QVariantMap>

class Workout;

class WorkoutSerializer
{
public:
    static constexpr const char *table = "workouts";
    static constexpr const char *id_key = "id";
    static constexpr const char *name_key = "name";
    static constexpr const char *created_time_key = "created_time";
    static constexpr const char *started_time_key = "started_time";
    static constexpr const char *ended_time_key = "ended_time";

    static Workout fromVariant(const QVariantMap &data);
    static QVariantMap toVariant(const Workout &workout);
};
