#pragma once

#include <QString>

enum class MuscleRole
{
    Primary,
    Secondary,
    Stabilizer
};

inline QString muscleRoleToString(MuscleRole role)
{
    switch (role)
    {
        case MuscleRole::Primary:
            return QStringLiteral("primary");
        case MuscleRole::Secondary:
            return QStringLiteral("secondary");
        case MuscleRole::Stabilizer:
            return QStringLiteral("stabilizer");
    }
    return QStringLiteral("primary");
}

inline MuscleRole muscleRoleFromString(const QString& str)
{
    if (str == QStringLiteral("secondary"))
        return MuscleRole::Secondary;
    if (str == QStringLiteral("stabilizer"))
        return MuscleRole::Stabilizer;
    return MuscleRole::Primary;
}
