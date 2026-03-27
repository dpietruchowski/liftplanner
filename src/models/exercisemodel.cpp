#include "exercisemodel.h"
#include <QJsonArray>

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

QJsonObject ExerciseModel::toJson(SerializationMode mode) const
{
    QJsonObject obj;
    obj["name"] = m_name;
    obj["rest_seconds"] = m_restSeconds;
    obj["youtube_link"] = m_youtubeLink;

    if (mode != SerializationMode::ChatGpt)
    {
        if (m_id != -1)
            obj["id"] = m_id;
        if (m_workoutId != -1)
            obj["workout_id"] = m_workoutId;
        obj["description"] = m_description;
    }

    if (mode == SerializationMode::ChatGpt)
    {
        obj["sets"] = setsToString();
    }
    else if (mode == SerializationMode::FullFile)
    {
        QJsonArray setsArray;
        for (QObject *setObj : m_sets)
        {
            if (auto *set = qobject_cast<SetModel *>(setObj))
                setsArray.append(set->toJson(mode));
        }
        obj["sets"] = setsArray;
    }

    return obj;
}

ExerciseModel *ExerciseModel::fromJson(const QJsonObject &jsonObj, QObject *parent)
{
    ExerciseModel *model = new ExerciseModel(parent);

    if (jsonObj.contains("id"))
        model->setId(jsonObj["id"].toInt());
    if (jsonObj.contains("workout_id"))
        model->setWorkoutId(jsonObj["workout_id"].toInt());
    if (jsonObj.contains("name"))
        model->setName(jsonObj["name"].toString());
    if (jsonObj.contains("description"))
        model->setDescription(jsonObj["description"].toString());
    if (jsonObj.contains("rest_seconds"))
        model->setRestSeconds(jsonObj["rest_seconds"].toInt());
    if (jsonObj.contains("youtube_link"))
        model->setYoutubeLink(jsonObj["youtube_link"].toString());

    if (jsonObj.contains("sets"))
    {
        QJsonValue setsVal = jsonObj["sets"];
        if (setsVal.isString())
        {
            model->setsFromString(setsVal.toString());
        }
        else if (setsVal.isArray())
        {
            QJsonArray setsArray = setsVal.toArray();
            for (const QJsonValue &val : setsArray)
            {
                SetModel *set = SetModel::fromJson(val.toObject(), model);
                if (set)
                    model->addSet(set);
            }
        }
    }

    return model;
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
