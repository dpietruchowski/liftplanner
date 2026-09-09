#pragma once

#include <QString>

enum class LoadType
{
    External,
    Bodyweight,
    Added,
    Assisted,
    Band,
    None
};

inline QString loadTypeToString(LoadType loadType)
{
    switch (loadType)
    {
        case LoadType::External:
            return QStringLiteral("external");
        case LoadType::Bodyweight:
            return QStringLiteral("bodyweight");
        case LoadType::Added:
            return QStringLiteral("added");
        case LoadType::Assisted:
            return QStringLiteral("assisted");
        case LoadType::Band:
            return QStringLiteral("band");
        case LoadType::None:
            return QStringLiteral("none");
    }
    return QStringLiteral("external");
}

inline LoadType loadTypeFromString(const QString& str)
{
    if (str == QStringLiteral("bodyweight"))
        return LoadType::Bodyweight;
    if (str == QStringLiteral("added"))
        return LoadType::Added;
    if (str == QStringLiteral("assisted"))
        return LoadType::Assisted;
    if (str == QStringLiteral("band"))
        return LoadType::Band;
    if (str == QStringLiteral("none"))
        return LoadType::None;
    return LoadType::External;
}
