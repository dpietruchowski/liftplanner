#pragma once

#include <QString>

enum class SetMetric
{
    Reps,
    Duration,
    Distance
};

inline QString setMetricToString(SetMetric metric)
{
    switch (metric)
    {
        case SetMetric::Reps:
            return QStringLiteral("reps");
        case SetMetric::Duration:
            return QStringLiteral("duration");
        case SetMetric::Distance:
            return QStringLiteral("distance");
    }
    return QStringLiteral("reps");
}

inline SetMetric setMetricFromString(const QString& str)
{
    if (str == QStringLiteral("duration"))
        return SetMetric::Duration;
    if (str == QStringLiteral("distance"))
        return SetMetric::Distance;
    return SetMetric::Reps;
}
