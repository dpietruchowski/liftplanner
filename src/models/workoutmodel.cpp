#include "workoutmodel.h"
#include <QJsonArray>
#include "../utils/serializationutils.h"

WorkoutModel::WorkoutModel(QObject *parent)
    : QObject(parent)
{
    m_id = -1;

    connect(this, &WorkoutModel::idChanged, this, &WorkoutModel::onIdUpdated);
}

void WorkoutModel::addExercise(ExerciseModel *exercise) {
    if (exercise && !m_exercises.contains(exercise)) {
        exercise->setParent(this);
        exercise->setWorkoutId(m_id);
        m_exercises.append(exercise);
    }
}

void WorkoutModel::removeExercise(ExerciseModel *exercise) {
    if (exercise && m_exercises.contains(exercise)) {
        m_exercises.removeAll(exercise);
        exercise->setParent(nullptr);
    }
}

QVariantMap WorkoutModel::toVariantMap(bool dbModel) const
{
    QVariantMap variant;
    if (m_id != -1) {
        variant.insert(WorkoutModel::id_key, m_id);
    }
    variant.insert(WorkoutModel::name_key, m_name);

    if (!dbModel) {
        QVariantList exercisesList;
        for (QObject *obj : m_exercises) {
            if (ExerciseModel *exercise = qobject_cast<ExerciseModel *>(obj)) {
                exercisesList.append(exercise->toVariantMap());
            }
        }
        variant.insert(WorkoutModel::exercises_key, exercisesList);
    }
    return variant;
}

WorkoutModel* WorkoutModel::fromVariantMap(const QVariantMap &variantMap, QObject *parent) {
    WorkoutModel *model = new WorkoutModel(parent);

    if (variantMap.contains(WorkoutModel::id_key)) model->setId(variantMap.value(WorkoutModel::id_key).toInt());
    if (variantMap.contains(WorkoutModel::name_key)) model->setName(variantMap.value(WorkoutModel::name_key).toString());

    if (variantMap.contains(WorkoutModel::exercises_key)) {
        QVariantList exercisesList = variantMap.value(WorkoutModel::exercises_key).toList();
        for (const QVariant &var : exercisesList) {
            if (var.canConvert<QVariantMap>()) {
                ExerciseModel *exercise = ExerciseModel::fromVariantMap(var.toMap(), model);
                model->addExercise(exercise);
            }
        }
    }

    return model;
}

QJsonObject WorkoutModel::toJson() const {
    return Serialization::toJson(toVariantMap());
}

WorkoutModel* WorkoutModel::fromJson(const QJsonObject &jsonObj, QObject *parent) {
    return fromVariantMap(Serialization::fromJson(jsonObj), parent);
}

void WorkoutModel::onIdUpdated()
{
    for (QObject *exerciseObj : exercises()) {
        ExerciseModel *exercise = qobject_cast<ExerciseModel *>(exerciseObj);
        exercise->setWorkoutId(id());
    }
}
