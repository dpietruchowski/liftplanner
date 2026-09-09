#pragma once

#include <QString>

enum class UnitSystem
{
    Metric,
    Imperial
};

inline QString unitSystemToString(UnitSystem system)
{
    switch (system)
    {
        case UnitSystem::Metric:
            return QStringLiteral("metric");
        case UnitSystem::Imperial:
            return QStringLiteral("imperial");
    }
    return QStringLiteral("metric");
}

inline UnitSystem unitSystemFromString(const QString& str)
{
    if (str == QStringLiteral("imperial"))
        return UnitSystem::Imperial;
    return UnitSystem::Metric;
}
