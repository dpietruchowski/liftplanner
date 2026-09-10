#pragma once

#include "workout.h"
#include <QDateTime>

inline Workout asPlanned(const Workout& source)
{
    Workout planned = source;
    planned.setStatus(WorkoutStatus::Planned);
    planned.setStartedTime(QDateTime());
    planned.setEndedTime(QDateTime());

    for (Exercise& exercise : planned.exercises())
    {
        for (Set& set : exercise.sets())
            set.setCompleted(false);
    }

    return planned;
}
