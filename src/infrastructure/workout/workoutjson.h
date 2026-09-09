#pragma once

#include "domain/workout/exercise.h"
#include "domain/workout/set.h"
#include "domain/workout/workout.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

namespace WorkoutJson
{

// Full JSON (for file save/load)
QJsonObject setToJson(const Set& set);
QJsonObject exerciseToJson(const Exercise& exercise);
QJsonObject workoutToJson(const Workout& workout);

// Compact JSON (for GPT prompt)
QJsonObject exerciseToJsonCompact(const Exercise& exercise);
QJsonObject workoutToJsonCompact(const Workout& workout);

// Compact sets grammar, e.g. "10x80kg, 12xBW+5kg, 45s, 5km@24min, 8x(20s/10s)"
std::vector<Set> parseSets(const QString& text, QStringList* errors = nullptr);

// Parsing (handles both full and compact formats)
Set setFromJson(const QJsonObject& json);
Exercise exerciseFromJson(const QJsonObject& json, QStringList* errors = nullptr);
Workout workoutFromJson(const QJsonObject& json, QStringList* errors = nullptr);
std::vector<Workout> workoutsFromJsonArray(const QJsonArray& array, QStringList* errors = nullptr);

}  // namespace WorkoutJson
