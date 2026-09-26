#pragma once

#include "async/timeprovider.h"
#include "generatedworkoutname.h"
#include "setprescription.h"
#include "workout.h"

inline Exercise repeatOf(const Exercise& source)
{
    Exercise repeated = source;
    repeated.setId(-1);
    repeated.setWorkoutId(-1);
    repeated.sets().clear();

    for (const Set& set : source.sets())
        repeated.addSet(SetPrescription::fromSet(set).toSet());

    return repeated;
}

inline Workout repeatOf(const Workout& source, const QDateTime& plannedTime)
{
    Workout repeated(source.hasGeneratedName() ? generatedPlanName() : source.name(),
                     TimeProvider::instance().currentDateTime());
    repeated.setGeneratedName(source.hasGeneratedName());
    repeated.setPlannedTime(plannedTime);
    repeated.setStatus(WorkoutStatus::Planned);

    for (const Exercise& exercise : source.exercises())
        repeated.addExercise(repeatOf(exercise));

    repeated.normalizePositions();
    return repeated;
}
