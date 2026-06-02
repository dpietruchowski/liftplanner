#pragma once

#include "modules/workout/domain/entities/workout.h"
#include <QString>

namespace WorkoutText
{

// Human-readable single-workout export (export only, no parsing):
//   <name>,<yyyy-MM-dd>
//
//   <exercise>,<rep>x<weight>kg,...
QString workoutToText(const Workout& workout);

}  // namespace WorkoutText
