#pragma once

#include "performedsets.h"
#include "strengthmath.h"
#include "workout.h"

struct SessionSummary
{
    int completedSets { 0 };
    int plannedSets { 0 };
    double volume { 0.0 };
    qint64 durationSeconds { 0 };

    bool isEmpty() const { return completedSets == 0; }
};

inline qint64 sessionDurationSeconds(const Workout& workout)
{
    if (!workout.startedTime().isValid() || !workout.endedTime().isValid())
        return 0;

    const qint64 elapsed = workout.startedTime().secsTo(workout.endedTime());
    return elapsed > 0 ? elapsed : 0;
}

inline SessionSummary summarizeSession(const Workout& workout)
{
    const bool flagsAreMeaningful = completionFlagsAreMeaningful(workout);

    SessionSummary summary;
    for (const Exercise& exercise : workout.exercises())
    {
        for (const Set& set : exercise.sets())
        {
            summary.plannedSets += 1;

            if (!countsAsPerformed(true, set.completed(), flagsAreMeaningful))
                continue;

            summary.completedSets += 1;

            if (StrengthMath::isWeighted(set.metric(), set.loadType()))
                summary.volume += StrengthMath::volume(set.repetitions(), set.weight());
        }
    }

    summary.durationSeconds = sessionDurationSeconds(workout);
    return summary;
}
