#include "workoutmodel.h"
#include <QJsonArray>

WorkoutModel::WorkoutModel(QObject *parent)
    : QObject(parent)
{
    m_id = -1;
    m_createdTime = QDateTime::currentDateTime();

    connect(this, &WorkoutModel::idChanged, this, &WorkoutModel::onIdUpdated);
}

bool WorkoutModel::isCompleted() const
{
    for (auto *s : m_exercises)
    {
        if (!s->isCompleted())
            return false;
    }
    return true;
}

void WorkoutModel::addExercise(ExerciseModel *exercise)
{
    if (exercise && !m_exercises.contains(exercise))
    {
        exercise->setParent(this);
        exercise->setWorkoutId(m_id);
        m_exercises.append(exercise);

        connect(exercise, &ExerciseModel::completedChanged, this, &WorkoutModel::completedChanged);

        emit exercisesChanged();
        emit completedChanged();
    }
}

void WorkoutModel::removeExercise(ExerciseModel *exercise)
{
    if (exercise && m_exercises.contains(exercise))
    {
        m_exercises.removeAll(exercise);
        exercise->setParent(nullptr);
    }
}

QJsonObject WorkoutModel::toJson(SerializationMode mode) const
{
    QJsonObject obj;
    if (m_id != -1)
        obj["id"] = m_id;
    obj["name"] = m_name;

    if (m_createdTime.isValid())
        obj["created_time"] = m_createdTime.toString(Qt::ISODate);
    if (m_startedTime.isValid())
        obj["started_time"] = m_startedTime.toString(Qt::ISODate);
    if (m_endedTime.isValid())
        obj["ended_time"] = m_endedTime.toString(Qt::ISODate);

    if (mode != SerializationMode::DbModel)
    {
        QJsonArray exercisesArray;
        for (ExerciseModel *exercise : m_exercises)
            exercisesArray.append(exercise->toJson(mode));
        obj["exercises"] = exercisesArray;
    }
    return obj;
}

WorkoutModel *WorkoutModel::fromJson(const QJsonObject &jsonObj, QObject *parent)
{
    WorkoutModel *model = new WorkoutModel(parent);

    if (jsonObj.contains("id"))
        model->setId(jsonObj["id"].toInt());
    if (jsonObj.contains("name"))
        model->setName(jsonObj["name"].toString());
    if (jsonObj.contains("created_time"))
        model->setCreatedTime(QDateTime::fromString(jsonObj["created_time"].toString(), Qt::ISODate));
    if (jsonObj.contains("started_time"))
        model->setStartedTime(QDateTime::fromString(jsonObj["started_time"].toString(), Qt::ISODate));
    if (jsonObj.contains("ended_time"))
        model->setEndedTime(QDateTime::fromString(jsonObj["ended_time"].toString(), Qt::ISODate));

    if (jsonObj.contains("exercises"))
    {
        QJsonArray exercisesArray = jsonObj["exercises"].toArray();
        for (const QJsonValue &val : exercisesArray)
        {
            ExerciseModel *exercise = ExerciseModel::fromJson(val.toObject(), model);
            if (exercise)
                model->addExercise(exercise);
        }
    }

    return model;
}

WorkoutModel *WorkoutModel::clone(QObject *parent) const
{
    WorkoutModel *clone = new WorkoutModel(parent);
    clone->setName(m_name);
    clone->setCreatedTime(m_createdTime);
    clone->setStartedTime(m_startedTime);
    clone->setEndedTime(m_endedTime);

    for (ExerciseModel *exercise : m_exercises)
    {
        ExerciseModel *exerciseClone = exercise->clone(clone);
        clone->addExercise(exerciseClone);
    }

    return clone;
}

void WorkoutModel::onIdUpdated()
{
    for (ExerciseModel *exercise : m_exercises)
    {
        exercise->setWorkoutId(id());
    }
}
