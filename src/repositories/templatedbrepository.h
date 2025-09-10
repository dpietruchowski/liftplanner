#pragma once
#include <QVector>
#include "genericdbrepository.h"
#include "models/types.h"

template <typename ModelType>
class TemplateDbRepository : public GenericDbRepository
{
public:
    using GenericDbRepository::GenericDbRepository;

    ModelType *loadModel()
    {
        QVariantMap map = GenericDbRepository::loadModel();
        return ModelType::fromVariantMap(map);
    }

    int add(const ModelType *item)
    {
        return GenericDbRepository::add(item->toVariantMap(SerializationMode::DbModel));
    }
    int save(const ModelType *item)
    {
        return GenericDbRepository::save(item->toVariantMap(SerializationMode::DbModel));
    }
    void saveAll(const QVector<ModelType *> &items)
    {
        QVector<QVariantMap> maps;
        for (const auto *item : items)
            maps.append(item->toVariantMap(true));
        GenericDbRepository::saveAll(maps);
    }

    QVector<ModelType *> loadAll(const QString &condition = QString())
    {
        QVector<ModelType *> models;
        QVector<QVariantMap> maps = GenericDbRepository::loadAll(condition);
        for (const auto& map : maps)
            models.append(ModelType::fromVariantMap(map));
        return models;
    }

private:
    QString m_tableName;
};
