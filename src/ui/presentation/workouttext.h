#pragma once

#include "domain/workout/workout.h"
#include <QDate>
#include <QString>
#include <QStringList>

namespace WorkoutText
{

// Human-readable single-workout export (export only, no parsing):
//   <name>,<yyyy-MM-dd>
//
//   <exercise>,<rep>x<weight>kg,...
QString workoutToText(const Workout& workout);

// Stat-tile formatting: "45s" / "12m" / "1h 05m", "800 m" / "5 km" / "12.4 km",
// "900 kg" / "12 400 kg".
QString finishPrompt(int completedSets, int plannedSets);
QString abandonPrompt(int exerciseCount);
QString amnestyLossWarning(const QString& workoutName, int plannedSets);
QString plannedDayLabel(const QDate& day, const QDate& today);
QStringList previousSetHints(const Exercise& previousPerformance, int currentSetCount);

QString formatDuration(qint64 seconds);
QString formatRest(int seconds);
QString formatDistance(double meters);
QString formatVolume(double kilograms);

}  // namespace WorkoutText
