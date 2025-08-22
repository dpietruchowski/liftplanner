#include "exercisemodel.h"
#include <QJsonArray>
#include "../utils/serializationutils.h"

ExerciseModel::ExerciseModel(QObject *parent)
{
    m_id = -1;
    m_workoutId = -1;

    connect(this, &ExerciseModel::idChanged, this, &ExerciseModel::onIdUpdated);
}

void ExerciseModel::addSet(SetModel *set)
{
    if (set && !m_sets.contains(set)) {
        set->setParent(this);
        set->setExerciseId(m_id);
        m_sets.append(set);
    }
}

void ExerciseModel::removeSet(SetModel *set) {
    if (set && m_sets.contains(set)) {
        m_sets.removeAll(set);
        set->setParent(nullptr);
    }
}

QVariantMap ExerciseModel::toVariantMap(bool dbModel) const
{
    QVariantMap variant;
    if (m_id != -1) {
        variant.insert(ExerciseModel::id_key, m_id);
    }
    if (m_workoutId != -1) {
        variant.insert(ExerciseModel::workout_id_key, m_workoutId);
    }
    variant.insert(ExerciseModel::name_key, m_name);

    if (!dbModel) {
        QVariantList setsList;
        for (QObject *obj : m_sets) {
            if (SetModel *set = qobject_cast<SetModel *>(obj)) {
                setsList.append(set->toVariantMap());
            }
        }
        variant.insert(ExerciseModel::sets_key, setsList);
    }
    return variant;
}

ExerciseModel* ExerciseModel::fromVariantMap(const QVariantMap &variantMap, QObject *parent) {
    ExerciseModel *model = new ExerciseModel(parent);

    if (variantMap.contains(ExerciseModel::id_key)) model->setId(variantMap.value(ExerciseModel::id_key).toInt());
    if (variantMap.contains(ExerciseModel::workout_id_key)) model->setWorkoutId(variantMap.value(ExerciseModel::workout_id_key).toInt());
    if (variantMap.contains(ExerciseModel::name_key)) model->setName(variantMap.value(ExerciseModel::name_key).toString());

    if (variantMap.contains(ExerciseModel::sets_key)) {
        QVariantList setsList = variantMap.value(ExerciseModel::sets_key).toList();
        for (const QVariant &var : setsList) {
            if (var.canConvert<QVariantMap>()) {
                SetModel *set = SetModel::fromVariantMap(var.toMap(), model);
                model->addSet(set);
            }
        }
    }

    return model;
}

QJsonObject ExerciseModel::toJson() const {
    return Serialization::toJson(toVariantMap());
}

ExerciseModel* ExerciseModel::fromJson(const QJsonObject &jsonObj, QObject *parent) {
    return fromVariantMap(Serialization::fromJson(jsonObj), parent);
}

void ExerciseModel::onIdUpdated()
{
    for (QObject *setObj : sets()) {
        SetModel *set = qobject_cast<SetModel *>(setObj);
        set->setExerciseId(id());
    }
}
