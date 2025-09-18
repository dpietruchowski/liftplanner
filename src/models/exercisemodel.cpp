#include "exercisemodel.h"
#include <QJsonArray>
#include "utils/serializationutils.h"
#include "utils/variantmapvalidator.h"

ExerciseModel::ExerciseModel(QObject *parent)
    : QObject(parent)
{
    m_id = -1;
    m_workoutId = -1;
    m_restSeconds = 120;
}

bool ExerciseModel::isCompleted() const
{
    for (auto *s : m_sets)
    {
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

        emit setsChanged();
        emit completedChanged();
    }
}

QVariantMap ExerciseModel::toVariantMap(SerializationMode mode) const
{
    QVariantMap variant;

    variant.insert(ExerciseModel::name_key, m_name);
    variant.insert(ExerciseModel::rest_seconds_key, m_restSeconds);
    variant.insert(ExerciseModel::youtube_link_key, m_youtubeLink);

    if (mode != SerializationMode::ChatGpt) {
        if (m_id != -1) {
            variant.insert(ExerciseModel::id_key, m_id);
        }
        if (m_workoutId != -1) {
            variant.insert(ExerciseModel::workout_id_key, m_workoutId);
        }
        variant.insert(ExerciseModel::description_key, m_description);
    }

    if (mode == SerializationMode::ChatGpt) {
        variant.insert(ExerciseModel::sets_key, setsToString());
    } else if (mode == SerializationMode::FullFile) {
        QVariantList setsList;
        for (SetModel *set : m_sets) {
            setsList.append(set->toVariantMap(mode));
        }
        variant.insert(ExerciseModel::sets_key, setsList);
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

    if (variantMap.contains(ExerciseModel::description_key))
        model->setDescription(variantMap.value(ExerciseModel::description_key).toString());

    if (variantMap.contains(ExerciseModel::rest_seconds_key))
        model->setRestSeconds(variantMap.value(ExerciseModel::rest_seconds_key).toInt());
    if (variantMap.contains(ExerciseModel::youtube_link_key))
        model->setYoutubeLink(variantMap.value(ExerciseModel::youtube_link_key).toString());

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

bool ExerciseModel::validateVariantMap(const QVariantMap &variantMap, QString &stringError)
{
    VariantMapValidator validator(variantMap, stringError);

    if (!validator.validateString(ExerciseModel::name_key, true))
        return false;
    if (!validator.validateInt(ExerciseModel::rest_seconds_key, true))
        return false;
    if (!validator.validateString(ExerciseModel::description_key, false))
        return false;
    if (!validator.validateUrl(ExerciseModel::youtube_link_key, false))
        return false;

    if (variantMap.contains(ExerciseModel::sets_key)) {
        QVariant setsVar = variantMap.value(ExerciseModel::sets_key);

        if (setsVar.typeId() == QMetaType::QVariantList) {
            QVariantList setsList = setsVar.toList();
            for (int i = 0; i < setsList.size(); ++i) {
                const QVariant &var = setsList[i];
                if (!var.canConvert<QVariantMap>()) {
                    stringError = QString("Set at index %1 is not a valid object").arg(i);
                    return false;
                }
                QString setError;
                if (!SetModel::validateVariantMap(var.toMap(), setError)) {
                    stringError = QString("Set at index %1 invalid: %2").arg(i).arg(setError);
                    return false;
                }
            }
        } else if (setsVar.typeId() == QMetaType::QString) {
            QString setsStr = setsVar.toString();
            QStringList setStrings = setsStr.split(",", Qt::SkipEmptyParts);
            for (int i = 0; i < setStrings.size(); ++i) {
                const QString &s = setStrings[i].trimmed();
                QString setError;
                if (!SetModel::validateString(s, setError)) {
                    stringError = QString("Set at index %1 invalid: %2").arg(i).arg(setError);
                    return false;
                }
            }
        } else {
            stringError = "'sets' field has invalid type";
            return false;
        }
    }

    stringError.clear();
    return true;
}

QJsonObject ExerciseModel::toJson(SerializationMode mode) const
{
    return Serialization::toJson(toVariantMap(mode));
}

ExerciseModel *ExerciseModel::fromJson(const QJsonObject &jsonObj, QObject *parent)
{
    return fromVariantMap(Serialization::fromJson(jsonObj), parent);
}

QString ExerciseModel::setsToString() const
{
    QStringList setStrings;
    for (QObject *obj : m_sets)
    {
        if (SetModel *set = qobject_cast<SetModel *>(obj))
        {
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
    for (QString s : setStrings)
    {
        if (SetModel *set = SetModel::fromString(s.trimmed(), this))
        {
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
    clone->setDescription(m_description);
    clone->setYoutubeLink(m_youtubeLink);

    for (QObject *setObj : m_sets) {
        if (SetModel *set = qobject_cast<SetModel *>(setObj))
        {
            SetModel *setClone = set->clone(clone);
            clone->addSet(setClone);
        }
    }

    return clone;
}
