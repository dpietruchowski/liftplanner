#pragma once

#include <QVariantMap>
#include <QVector>
#include <type_traits>
#include <vector>

namespace DbRows
{

template <typename Convert> auto toEntities(const QVector<QVariantMap>& rows, Convert convert)
{
    using Entity = std::decay_t<decltype(convert(rows.first()))>;

    std::vector<Entity> entities;
    entities.reserve(rows.size());
    for (const auto& row : rows)
        entities.push_back(convert(row));
    return entities;
}

}
