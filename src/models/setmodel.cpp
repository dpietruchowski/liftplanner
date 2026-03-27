#include "setmodel.h"

SetModel::SetModel(QObject *parent)
    : QObject(parent)
{
    m_id = -1;
    m_exerciseId = -1;
}

QJsonObject SetModel::toJson(SerializationMode) const
{
    QJsonObject obj;
    if (m_id != -1)
        obj["id"] = m_id;
    if (m_exerciseId != -1)
        obj["exercise_id"] = m_exerciseId;
    obj["repetitions"] = m_repetitions;
    obj["weight"] = m_weight;
    obj["completed"] = m_completed;
    return obj;
}

SetModel *SetModel::fromJson(const QJsonObject &jsonObj, QObject *parent)
{
    SetModel *model = new SetModel(parent);

    if (jsonObj.contains("id"))
        model->setId(jsonObj["id"].toInt());
    if (jsonObj.contains("exercise_id"))
        model->setExerciseId(jsonObj["exercise_id"].toInt());
    if (jsonObj.contains("repetitions"))
        model->setRepetitions(jsonObj["repetitions"].toInt());
    if (jsonObj.contains("weight"))
        model->setWeight(jsonObj["weight"].toInt());
    if (jsonObj.contains("completed"))
        model->setCompleted(jsonObj["completed"].toBool());

    return model;
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

SetModel *SetModel::clone(QObject *parent) const
{
    SetModel *clone = new SetModel(parent);
    clone->setExerciseId(m_exerciseId);
    clone->setRepetitions(m_repetitions);
    clone->setWeight(m_weight);
    clone->setCompleted(m_completed);
    return clone;
}
