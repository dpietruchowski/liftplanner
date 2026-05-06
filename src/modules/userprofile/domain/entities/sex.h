#pragma once

#include <QString>

enum class Sex
{
    Male,
    Female,
    Other
};

inline QString sexToString(Sex sex)
{
    switch (sex)
    {
    case Sex::Male:   return QStringLiteral("male");
    case Sex::Female: return QStringLiteral("female");
    case Sex::Other:  return QStringLiteral("other");
    }
    return QStringLiteral("other");
}

inline Sex sexFromString(const QString &str)
{
    if (str == QStringLiteral("male"))   return Sex::Male;
    if (str == QStringLiteral("female")) return Sex::Female;
    return Sex::Other;
}
