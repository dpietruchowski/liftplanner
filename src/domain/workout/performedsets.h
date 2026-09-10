#pragma once

#include "exercise.h"
#include "workout.h"
#include <algorithm>

inline bool hasCompletedSet(const Exercise& exercise)
{
    const std::vector<Set>& sets = exercise.sets();
    return std::any_of(sets.begin(), sets.end(), [](const Set& set) { return set.completed(); });
}

inline bool completionFlagsAreMeaningful(const Workout& workout)
{
    const std::vector<Exercise>& exercises = workout.exercises();
    return std::any_of(exercises.begin(), exercises.end(),
                       [](const Exercise& exercise) { return hasCompletedSet(exercise); });
}

inline bool wasPerformed(const Exercise& exercise, bool flagsAreMeaningful)
{
    return flagsAreMeaningful ? hasCompletedSet(exercise) : !exercise.sets().empty();
}

inline Exercise asPerformed(const Exercise& exercise, bool flagsAreMeaningful)
{
    if (flagsAreMeaningful)
        return exercise;

    Exercise performed = exercise;
    for (Set& set : performed.sets())
        set.setCompleted(true);

    return performed;
}
