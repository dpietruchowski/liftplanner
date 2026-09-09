#pragma once

#include <QString>

enum class ExperienceLevel
{
    Beginner,
    Intermediate,
    Advanced
};

inline QString experienceLevelToString(ExperienceLevel level)
{
    switch (level)
    {
        case ExperienceLevel::Beginner:
            return QStringLiteral("beginner");
        case ExperienceLevel::Intermediate:
            return QStringLiteral("intermediate");
        case ExperienceLevel::Advanced:
            return QStringLiteral("advanced");
    }
    return QStringLiteral("beginner");
}

inline ExperienceLevel experienceLevelFromString(const QString& str)
{
    if (str == QStringLiteral("intermediate"))
        return ExperienceLevel::Intermediate;
    if (str == QStringLiteral("advanced"))
        return ExperienceLevel::Advanced;
    return ExperienceLevel::Beginner;
}
