#include "utils/serializationutils.h"

#include <QJsonDocument>
#include <QJsonValue>

namespace Serialization
{
    QJsonObject toJson(const QVariantMap &variantMap)
    {
        QJsonObject obj;
        for (auto it = variantMap.begin(); it != variantMap.end(); ++it)
        {
            obj.insert(it.key(), QJsonValue::fromVariant(it.value()));
        }
        return obj;
    }

    QVariantMap fromJson(const QJsonObject &jsonObj)
    {
        QVariantMap variantMap;
        for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it)
        {
            variantMap.insert(it.key(), it.value().toVariant());
        }
        return variantMap;
    }
}
