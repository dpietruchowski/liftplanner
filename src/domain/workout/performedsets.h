#pragma once

#include "exercise.h"
#include "historysetrow.h"
#include "workout.h"
#include <QSet>
#include <algorithm>
#include <vector>

inline bool countsAsPerformed(bool hasContent, bool ticked, bool flagsAreMeaningful)
{
    return hasContent && (!flagsAreMeaningful || ticked);
}

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

inline QSet<int> sessionsWithMeaningfulFlags(const std::vector<HistorySetRow>& rows)
{
    QSet<int> sessions;
    for (const HistorySetRow& row : rows)
    {
        if (row.hasSet && row.completed)
            sessions.insert(row.workoutId);
    }

    return sessions;
}

inline bool wasPerformed(const Exercise& exercise, bool flagsAreMeaningful)
{
    return countsAsPerformed(!exercise.sets().empty(), hasCompletedSet(exercise),
                             flagsAreMeaningful);
}

inline bool wasPerformed(const HistorySetRow& row, bool flagsAreMeaningful)
{
    return countsAsPerformed(row.hasSet, row.completed, flagsAreMeaningful);
}

inline std::vector<Set> performedSets(const Exercise& exercise)
{
    std::vector<Set> done;
    for (const Set& set : exercise.sets())
    {
        if (set.completed())
            done.push_back(set);
    }

    return done;
}

inline Workout trimmedToPerformed(const Workout& workout)
{
    const bool flagsAreMeaningful = completionFlagsAreMeaningful(workout);
    if (!flagsAreMeaningful)
        return workout;

    std::vector<Exercise> kept;
    for (const Exercise& exercise : workout.exercises())
    {
        if (!wasPerformed(exercise, flagsAreMeaningful))
            continue;

        Exercise done = exercise;
        done.sets() = performedSets(exercise);
        kept.push_back(done);
    }

    Workout trimmed = workout;
    trimmed.exercises() = kept;
    trimmed.normalizePositions();
    return trimmed;
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
