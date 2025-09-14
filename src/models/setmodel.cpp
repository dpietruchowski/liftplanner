#include "setmodel.h"
#include "../utils/serializationutils.h"

SetModel::SetModel(QObject *parent)
    : QObject(parent)
{
    m_id = -1;
    m_exerciseId = -1;
}

QVariantMap SetModel::toVariantMap(SerializationMode) const
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

bool SetModel::validateVariantMap(const QVariantMap &variantMap, QString &stringError)
{
    if (!variantMap.contains(SetModel::repetitions_key)
        || !variantMap.value(SetModel::repetitions_key).canConvert<int>()) {
        stringError = QString("Missing or invalid '%1'").arg(SetModel::repetitions_key);
        return false;
    }

    int repetitions = variantMap.value(SetModel::repetitions_key).toInt();
    if (repetitions <= 0) {
        stringError = QString("'%1' must be greater than 0").arg(SetModel::repetitions_key);
        return false;
    }

    if (!variantMap.contains(SetModel::weight_key)
        || !variantMap.value(SetModel::weight_key).canConvert<double>()) {
        stringError = QString("Missing or invalid '%1'").arg(SetModel::weight_key);
        return false;
    }

    double weight = variantMap.value(SetModel::weight_key).toDouble();
    if (weight <= 0) {
        stringError = QString("'%1' must be greater than 0").arg(SetModel::weight_key);
        return false;
    }

    if (!variantMap.contains(SetModel::completed_key)
        || !variantMap.value(SetModel::completed_key).canConvert<bool>()) {
        stringError = QString("Missing or invalid '%1'").arg(SetModel::completed_key);
        return false;
    }

    stringError.clear();
    return true;
}

QJsonObject SetModel::toJson(SerializationMode mode) const
{
    return Serialization::toJson(toVariantMap(mode));
}

SetModel *SetModel::fromJson(const QJsonObject &jsonObj, QObject *parent)
{
    return fromVariantMap(Serialization::fromJson(jsonObj), parent);
}

QString SetModel::toString() const
{
    return QString("%1x%2kg").arg(m_repetitions).arg(m_weight);
}

SetModel *SetModel::fromString(const QString &str, QObject *parent)
{
    QStringList parts = str.toLower().replace("kg", "").split("x");
    if (parts.size() != 2)
        return nullptr;

    bool ok1 = false;
    bool ok2 = false;

    int reps = parts[0].trimmed().toInt(&ok1);
    double w = parts[1].trimmed().toDouble(&ok2);

    if (!ok1 || !ok2)
        return nullptr;

    SetModel *model = new SetModel(parent);
    model->setRepetitions(reps);
    model->setWeight(w);
    return model;
}

bool SetModel::validateString(const QString &str, QString &stringError)
{
    QStringList parts = str.toLower().replace("kg", "").split("x");
    if (parts.size() != 2) {
        stringError = QString("Invalid format: '%1'. Expected format is '<reps>x<weight>kg'")
                          .arg(str);
        return false;
    }

    bool ok1 = false;
    bool ok2 = false;

    int reps = parts[0].trimmed().toInt(&ok1);
    double weight = parts[1].trimmed().toDouble(&ok2);

    if (!ok1) {
        stringError = QString("Invalid repetitions: '%1'").arg(parts[0].trimmed());
        return false;
    }

    if (!ok2) {
        stringError = QString("Invalid weight: '%1'").arg(parts[1].trimmed());
        return false;
    }

    stringError.clear();
    return true;
}

SetModel *SetModel::clone(QObject *parent) const
{
    SetModel *clone = new SetModel(parent);
    clone->setExerciseId(m_exerciseId);
    clone->setRepetitions(m_repetitions);
    clone->setWeight(m_weight);
    clone->setCompleted(m_completed);
    return clone;
}
