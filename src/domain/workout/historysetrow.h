#pragma once

#include "domain/workout/loadtype.h"
#include "domain/workout/setmetric.h"
#include <QString>

struct HistorySetRow final
{
    int workoutId { -1 };
    int exerciseId { -1 };
    QString exerciseName;
    bool hasSet { false };
    bool completed { false };
    SetMetric metric { SetMetric::Reps };
    LoadType loadType { LoadType::External };
    int repetitions { 0 };
    double weight { 0.0 };
    int durationSeconds { 0 };
    double distanceMeters { 0.0 };
};
