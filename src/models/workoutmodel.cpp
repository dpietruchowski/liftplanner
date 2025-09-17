#include "workoutmodel.h"
#include <QJsonArray>
#include "utils/serializationutils.h"
#include "utils/variantmapvalidator.h"

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

QVariantMap WorkoutModel::toVariantMap(SerializationMode mode) const
{
    QVariantMap variant;
    if (m_id != -1)
    {
        variant.insert(WorkoutModel::id_key, m_id);
    }
    variant.insert(WorkoutModel::name_key, m_name);

    if (m_createdTime.isValid())
    {
        variant.insert(WorkoutModel::created_time_key, m_createdTime.toString(Qt::ISODate));
    }
    if (m_startedTime.isValid())
    {
        variant.insert(WorkoutModel::started_time_key, m_startedTime.toString(Qt::ISODate));
    }
    if (m_endedTime.isValid())
    {
        variant.insert(WorkoutModel::ended_time_key, m_endedTime.toString(Qt::ISODate));
    }

    if (mode != SerializationMode::DbModel)
    {
        QVariantList exercisesList;
        for (ExerciseModel *exercise : m_exercises)
        {
            exercisesList.append(exercise->toVariantMap(mode));
        }
        variant.insert(WorkoutModel::exercises_key, exercisesList);
    }
    return variant;
}

WorkoutModel *WorkoutModel::fromVariantMap(const QVariantMap &variantMap, QObject *parent)
{
    WorkoutModel *model = new WorkoutModel(parent);

    if (variantMap.contains(WorkoutModel::id_key))
        model->setId(variantMap.value(WorkoutModel::id_key).toInt());
    if (variantMap.contains(WorkoutModel::name_key))
        model->setName(variantMap.value(WorkoutModel::name_key).toString());

    if (variantMap.contains(WorkoutModel::created_time_key))
    {
        model->setCreatedTime(QDateTime::fromString(variantMap.value(WorkoutModel::created_time_key).toString(), Qt::ISODate));
    }
    if (variantMap.contains(WorkoutModel::started_time_key))
    {
        model->setStartedTime(QDateTime::fromString(variantMap.value(WorkoutModel::started_time_key).toString(), Qt::ISODate));
    }
    if (variantMap.contains(WorkoutModel::ended_time_key))
    {
        model->setEndedTime(QDateTime::fromString(variantMap.value(WorkoutModel::ended_time_key).toString(), Qt::ISODate));
    }

    if (variantMap.contains(WorkoutModel::exercises_key))
    {
        QVariantList exercisesList = variantMap.value(WorkoutModel::exercises_key).toList();
        for (const QVariant &var : exercisesList)
        {
            if (var.canConvert<QVariantMap>())
            {
                ExerciseModel *exercise = ExerciseModel::fromVariantMap(var.toMap(), model);
                model->addExercise(exercise);
            }
        }
    }

    return model;
}

bool WorkoutModel::validateVariantMap(const QVariantMap &variantMap, QString &stringError)
{
    VariantMapValidator validator(variantMap, stringError);

    if (!validator.validateString(WorkoutModel::name_key, true))
        return false;

    if (variantMap.contains(WorkoutModel::exercises_key)) {
        QVariant exercisesVar = variantMap.value(WorkoutModel::exercises_key);
        if (exercisesVar.typeId() != QMetaType::QVariantList) {
            stringError = QString("Invalid type for '%1', expected list")
                              .arg(WorkoutModel::exercises_key);
            return false;
        }

        QVariantList exercisesList = exercisesVar.toList();
        for (int i = 0; i < exercisesList.size(); ++i) {
            const QVariant &var = exercisesList[i];
            if (!var.canConvert<QVariantMap>()) {
                stringError = QString("Exercise at index %1 is not a valid object").arg(i);
                return false;
            }

            QString exerciseError;
            if (!ExerciseModel::validateVariantMap(var.toMap(), exerciseError)) {
                stringError = QString("Exercise at index %1 invalid: %2").arg(i).arg(exerciseError);
                return false;
            }
        }
    }

    stringError.clear();
    return true;
}

QJsonObject WorkoutModel::toJson(SerializationMode mode) const
{
    return Serialization::toJson(toVariantMap(mode));
}

WorkoutModel *WorkoutModel::fromJson(const QJsonObject &jsonObj, QObject *parent)
{
    return fromVariantMap(Serialization::fromJson(jsonObj), parent);
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
