#pragma once

#include "exercise.h"
#include "set.h"
#include "setprescription.h"
#include <QtGlobal>
#include <algorithm>
#include <optional>

inline bool sameSetValues(const Set& a, const Set& b)
{
    return a.metric() == b.metric() && a.loadType() == b.loadType()
        && a.repetitions() == b.repetitions() && qFuzzyCompare(a.weight() + 1.0, b.weight() + 1.0)
        && a.durationSeconds() == b.durationSeconds()
        && qFuzzyCompare(a.distanceMeters() + 1.0, b.distanceMeters() + 1.0);
}

inline std::optional<Set> lastCompletedSet(const Exercise& exercise)
{
    const std::vector<Set>& sets = exercise.sets();
    const auto found
        = std::find_if(sets.rbegin(), sets.rend(), [](const Set& set) { return set.completed(); });
    if (found == sets.rend())
        return std::nullopt;

    return *found;
}

inline std::optional<Set> seedFromPreviousExercise(const Exercise& previous, const Set& fallback)
{
    const std::optional<Set> performed = lastCompletedSet(previous);
    if (!performed.has_value() || performed->metric() != fallback.metric())
        return std::nullopt;

    Set seeded = SetPrescription::fromSet(performed.value()).toSet();
    seeded.setPosition(fallback.position());
    seeded.setRestSecondsOverride(fallback.restSecondsOverride());

    if (sameSetValues(seeded, fallback))
        return std::nullopt;

    return seeded;
}
