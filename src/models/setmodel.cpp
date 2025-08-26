#include "setmodel.h"
#include "../utils/serializationutils.h"

QVariantMap SetModel::toVariantMap(bool dbModel) const
{
    QVariantMap variant;
    if (m_id != -1)
    {
        variant.insert(SetModel::id_key, m_id);
    }
    if (m_exerciseId != -1)
    {
        variant.insert(SetModel::exercise_id_key, m_exerciseId);
    }
    variant.insert(SetModel::repetitions_key, m_repetitions);
    variant.insert(SetModel::weight_key, m_weight);
    variant.insert(SetModel::completed_key, m_completed);
    return variant;
}

SetModel *SetModel::fromVariantMap(const QVariantMap &variantMap, QObject *parent)
{
    SetModel *model = new SetModel(parent);

    if (variantMap.contains(SetModel::id_key))
        model->setId(variantMap.value(SetModel::id_key).toInt());
    if (variantMap.contains(SetModel::exercise_id_key))
        model->setExerciseId(variantMap.value(SetModel::exercise_id_key).toInt());
    if (variantMap.contains(SetModel::repetitions_key))
        model->setRepetitions(variantMap.value(SetModel::repetitions_key).toInt());
    if (variantMap.contains(SetModel::weight_key))
        model->setWeight(variantMap.value(SetModel::weight_key).toInt());
    if (variantMap.contains(SetModel::completed_key))
        model->setCompleted(variantMap.value(SetModel::completed_key).toBool());

    return model;
}

QJsonObject SetModel::toJson() const
{
    return Serialization::toJson(toVariantMap());
}

SetModel *SetModel::fromJson(const QJsonObject &jsonObj, QObject *parent)
{
    return fromVariantMap(Serialization::fromJson(jsonObj), parent);
}

SetModel *SetModel::clone(QObject *parent) const
{
    SetModel *clone = new SetModel(parent);
    clone->setExerciseId(m_exerciseId);
    clone->setRepetitions(m_repetitions);
    clone->setWeight(m_weight);
    return clone;
}
