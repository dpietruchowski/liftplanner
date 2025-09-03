#include "exercisemodel.h"
#include <QJsonArray>
#include "../utils/serializationutils.h"

ExerciseModel::ExerciseModel(QObject *parent)
    : QObject(parent)
{
    m_id = -1;
    m_workoutId = -1;
    m_restSeconds = 120;
}

bool ExerciseModel::isCompleted() const
{
    for (auto *s : m_sets) {
        if (!s->completed())
            return false;
    }
    return true;
}

void ExerciseModel::addSet(SetModel *set)
{
    if (set && !m_sets.contains(set))
    {
        set->setParent(this);
        set->setExerciseId(m_id);
        m_sets.append(set);

        connect(set, &SetModel::completedChanged, this, &ExerciseModel::completedChanged);

        emit setsChanged();
        emit completedChanged();
    }
}

void ExerciseModel::removeSet(SetModel *set)
{
    if (set && m_sets.contains(set))
    {
        m_sets.removeAll(set);
        set->setParent(nullptr);
    }
}

QVariantMap ExerciseModel::toVariantMap(bool dbModel) const
{
    QVariantMap variant;
    if (m_id != -1)
    {
        variant.insert(ExerciseModel::id_key, m_id);
    }
    if (m_workoutId != -1)
    {
        variant.insert(ExerciseModel::workout_id_key, m_workoutId);
    }
    variant.insert(ExerciseModel::name_key, m_name);
    variant.insert(ExerciseModel::rest_seconds_key, m_restSeconds);

    if (!dbModel) {
        variant.insert(ExerciseModel::sets_key, setsToString());
    }

    return variant;
}

ExerciseModel *ExerciseModel::fromVariantMap(const QVariantMap &variantMap, QObject *parent)
{
    ExerciseModel *model = new ExerciseModel(parent);

    if (variantMap.contains(ExerciseModel::id_key))
        model->setId(variantMap.value(ExerciseModel::id_key).toInt());
    if (variantMap.contains(ExerciseModel::workout_id_key))
        model->setWorkoutId(variantMap.value(ExerciseModel::workout_id_key).toInt());
    if (variantMap.contains(ExerciseModel::name_key))
        model->setName(variantMap.value(ExerciseModel::name_key).toString());
    if (variantMap.contains(ExerciseModel::rest_seconds_key))
        model->setRestSeconds(variantMap.value(ExerciseModel::rest_seconds_key).toInt());

    if (variantMap.contains(ExerciseModel::sets_key)) {
        QVariant setsVar = variantMap.value(ExerciseModel::sets_key);

        if (setsVar.typeId() == QMetaType::QString) {
            model->setsFromString(setsVar.toString());
        } else if (setsVar.typeId() == QMetaType::QVariantList) {
            QVariantList setsList = setsVar.toList();
            for (const QVariant &var : setsList) {
                if (var.canConvert<QVariantMap>()) {
                    SetModel *set = SetModel::fromVariantMap(var.toMap(), model);
                    model->addSet(set);
                }
            }
        }
    }

    return model;
}

QJsonObject ExerciseModel::toJson() const
{
    return Serialization::toJson(toVariantMap());
}

ExerciseModel *ExerciseModel::fromJson(const QJsonObject &jsonObj, QObject *parent)
{
    return fromVariantMap(Serialization::fromJson(jsonObj), parent);
}

QString ExerciseModel::setsToString() const
{
    QStringList setStrings;
    for (QObject *obj : m_sets) {
        if (SetModel *set = qobject_cast<SetModel *>(obj)) {
            setStrings.append(set->toString());
        }
    }
    return setStrings.join(", ");
}

void ExerciseModel::setsFromString(const QString &str)
{
    qDeleteAll(m_sets);
    m_sets.clear();

    QStringList setStrings = str.split(",", Qt::SkipEmptyParts);
    for (QString s : setStrings) {
        if (SetModel *set = SetModel::fromString(s.trimmed(), this)) {
            m_sets.append(set);
        }
    }
}

ExerciseModel *ExerciseModel::clone(QObject *parent) const
{
    ExerciseModel *clone = new ExerciseModel(parent);
    clone->setName(m_name);
    clone->setWorkoutId(m_workoutId);
    clone->setRestSeconds(m_restSeconds);

    for (QObject *setObj : m_sets)
    {
        if (SetModel *set = qobject_cast<SetModel *>(setObj))
        {
            SetModel *setClone = set->clone(clone);
            clone->addSet(setClone);
        }
    }

    return clone;
}
