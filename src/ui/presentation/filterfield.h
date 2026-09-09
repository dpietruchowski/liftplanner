#pragma once

#include <QString>

namespace FilterField
{

inline bool apply(QString& target, const QString& value)
{
    const QString trimmed = value.trimmed();
    if (target == trimmed)
        return false;

    target = trimmed;
    return true;
}

}
