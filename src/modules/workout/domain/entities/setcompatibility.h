#pragma once

#include "exercisekind.h"
#include "loadtype.h"
#include "setmetric.h"

inline bool metricSuitsKind(ExerciseKind kind, SetMetric metric)
{
    switch (kind)
    {
        case ExerciseKind::Strength:
        case ExerciseKind::Bodyweight:
            return metric == SetMetric::Reps;
        case ExerciseKind::Cardio:
            return metric == SetMetric::Duration || metric == SetMetric::Distance;
        case ExerciseKind::Interval:
        case ExerciseKind::Mobility:
            return metric == SetMetric::Duration || metric == SetMetric::Reps;
        case ExerciseKind::Isometric:
            return metric == SetMetric::Duration;
    }
    return false;
}

inline SetMetric defaultMetricFor(ExerciseKind kind)
{
    switch (kind)
    {
        case ExerciseKind::Strength:
        case ExerciseKind::Bodyweight:
            return SetMetric::Reps;
        case ExerciseKind::Cardio:
            return SetMetric::Distance;
        case ExerciseKind::Interval:
        case ExerciseKind::Mobility:
        case ExerciseKind::Isometric:
            return SetMetric::Duration;
    }
    return SetMetric::Reps;
}

inline LoadType defaultLoadTypeFor(ExerciseKind kind)
{
    switch (kind)
    {
        case ExerciseKind::Strength:
            return LoadType::External;
        case ExerciseKind::Bodyweight:
            return LoadType::Bodyweight;
        case ExerciseKind::Cardio:
        case ExerciseKind::Interval:
        case ExerciseKind::Mobility:
        case ExerciseKind::Isometric:
            return LoadType::None;
    }
    return LoadType::External;
}
