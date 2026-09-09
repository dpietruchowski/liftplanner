#pragma once

#include <QString>

enum class CatalogOrigin
{
    BuiltIn,
    Custom,
    Imported
};

inline QString catalogOriginToString(CatalogOrigin origin)
{
    switch (origin)
    {
        case CatalogOrigin::BuiltIn:
            return QStringLiteral("built_in");
        case CatalogOrigin::Custom:
            return QStringLiteral("custom");
        case CatalogOrigin::Imported:
            return QStringLiteral("imported");
    }
    return QStringLiteral("built_in");
}

inline CatalogOrigin catalogOriginFromString(const QString& str)
{
    if (str == QStringLiteral("custom"))
        return CatalogOrigin::Custom;
    if (str == QStringLiteral("imported"))
        return CatalogOrigin::Imported;
    return CatalogOrigin::BuiltIn;
}
