#pragma once

#include <QString>

enum class Mechanics
{
    Compound,
    Isolation
};

inline QString mechanicsToString(Mechanics mechanics)
{
    switch (mechanics)
    {
        case Mechanics::Compound:
            return QStringLiteral("compound");
        case Mechanics::Isolation:
            return QStringLiteral("isolation");
    }
    return QStringLiteral("compound");
}

inline Mechanics mechanicsFromString(const QString& str)
{
    if (str == QStringLiteral("isolation"))
        return Mechanics::Isolation;
    return Mechanics::Compound;
}
