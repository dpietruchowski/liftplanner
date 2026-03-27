#pragma once

#include <QString>

enum class WorkoutStatus
{
    Planned,
    Started,
    Ended
};

inline QString workoutStatusToString(WorkoutStatus status)
{
    switch (status)
    {
    case WorkoutStatus::Planned:
        return QStringLiteral("Planned");
    case WorkoutStatus::Started:
        return QStringLiteral("Started");
    case WorkoutStatus::Ended:
        return QStringLiteral("Ended");
    }
    return QStringLiteral("Planned");
}

inline WorkoutStatus workoutStatusFromString(const QString &str)
{
    if (str == QStringLiteral("Started"))
        return WorkoutStatus::Started;
    if (str == QStringLiteral("Ended"))
        return WorkoutStatus::Ended;
    return WorkoutStatus::Planned;
}
