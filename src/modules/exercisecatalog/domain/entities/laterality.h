#pragma once

#include <QString>

enum class Laterality
{
    Bilateral,
    Unilateral
};

inline QString lateralityToString(Laterality laterality)
{
    switch (laterality)
    {
        case Laterality::Bilateral:
            return QStringLiteral("bilateral");
        case Laterality::Unilateral:
            return QStringLiteral("unilateral");
    }
    return QStringLiteral("bilateral");
}

inline Laterality lateralityFromString(const QString& str)
{
    if (str == QStringLiteral("unilateral"))
        return Laterality::Unilateral;
    return Laterality::Bilateral;
}
