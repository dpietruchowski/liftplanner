#pragma once

#include <QString>

enum class ExerciseKind
{
    Strength,
    Bodyweight,
    Cardio,
    Interval,
    Mobility,
    Isometric
};

inline QString exerciseKindToString(ExerciseKind kind)
{
    switch (kind)
    {
        case ExerciseKind::Strength:
            return QStringLiteral("strength");
        case ExerciseKind::Bodyweight:
            return QStringLiteral("bodyweight");
        case ExerciseKind::Cardio:
            return QStringLiteral("cardio");
        case ExerciseKind::Interval:
            return QStringLiteral("interval");
        case ExerciseKind::Mobility:
            return QStringLiteral("mobility");
        case ExerciseKind::Isometric:
            return QStringLiteral("isometric");
    }
    return QStringLiteral("strength");
}

inline ExerciseKind exerciseKindFromString(const QString& str)
{
    if (str == QStringLiteral("bodyweight"))
        return ExerciseKind::Bodyweight;
    if (str == QStringLiteral("cardio"))
        return ExerciseKind::Cardio;
    if (str == QStringLiteral("interval"))
        return ExerciseKind::Interval;
    if (str == QStringLiteral("mobility"))
        return ExerciseKind::Mobility;
    if (str == QStringLiteral("isometric"))
        return ExerciseKind::Isometric;
    return ExerciseKind::Strength;
}
