#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"

namespace WorkoutJson
{

    // Full JSON (for file save/load)
    QJsonObject setToJson(const Set &set);
    QJsonObject exerciseToJson(const Exercise &exercise);
    QJsonObject workoutToJson(const Workout &workout);

    // Compact JSON (for GPT prompt)
    QJsonObject exerciseToJsonCompact(const Exercise &exercise);
    QJsonObject workoutToJsonCompact(const Workout &workout);

    // Parsing (handles both full and compact formats)
    Set setFromJson(const QJsonObject &json);
    Exercise exerciseFromJson(const QJsonObject &json);
    Workout workoutFromJson(const QJsonObject &json);
    std::vector<Workout> workoutsFromJsonArray(const QJsonArray &array);

} // namespace WorkoutJson
