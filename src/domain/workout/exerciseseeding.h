#pragma once

#include "exercise.h"
#include "exercisekind.h"
#include "loadtype.h"
#include "set.h"
#include "setcompatibility.h"
#include "setmetric.h"
#include <QString>

namespace ExerciseSeeding
{
inline constexpr int repetitions = 8;
inline constexpr int durationSeconds = 30;
inline constexpr double distanceMeters = 1000.0;
}

inline Set seedSet(SetMetric metric, LoadType loadType)
{
    Set set;
    set.setMetric(metric);
    set.setLoadType(loadType);

    switch (metric)
    {
        case SetMetric::Reps:
            set.setRepetitions(ExerciseSeeding::repetitions);
            break;
        case SetMetric::Duration:
            set.setDurationSeconds(ExerciseSeeding::durationSeconds);
            break;
        case SetMetric::Distance:
            set.setDistanceMeters(ExerciseSeeding::distanceMeters);
            break;
    }

    return set;
}

inline Set seedSetForKind(ExerciseKind kind)
{
    return seedSet(defaultMetricFor(kind), defaultLoadTypeFor(kind));
}

inline Set seedSetForDefaults(ExerciseKind kind, SetMetric preferredMetric, LoadType loadType)
{
    const SetMetric metric
        = metricSuitsKind(kind, preferredMetric) ? preferredMetric : defaultMetricFor(kind);
    return seedSet(metric, loadType);
}

inline Exercise seededExercise(int definitionId, const QString& name, ExerciseKind kind,
                               int restSeconds, SetMetric preferredMetric, LoadType loadType)
{
    Exercise exercise = Exercise::createFromDefinition(definitionId, name, kind, restSeconds);
    exercise.addSet(seedSetForDefaults(kind, preferredMetric, loadType));
    return exercise;
}
