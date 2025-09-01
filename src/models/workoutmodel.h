#pragma once
#include <QObject>
#include <QVariantMap>
#include <QJsonObject>
#include <QList>
#include <QDateTime>
#include "exercisemodel.h"
#include "../utils/serializationutils.h"

class WorkoutModel : public QObject
{
    Q_OBJECT

    DECLARE_MODEL_PROPERTY(int, id, setId, id)
    DECLARE_MODEL_PROPERTY(QString, name, setName, name)
    DECLARE_MODEL_LIST_PROPERTY(WorkoutModel, ExerciseModel, exercises, setExercises, exercises)
    DECLARE_MODEL_PROPERTY(QDateTime, createdTime, setCreatedTime, created_time)
    DECLARE_MODEL_PROPERTY(QDateTime, startedTime, setStartedTime, started_time)
    DECLARE_MODEL_PROPERTY(QDateTime, endedTime, setEndedTime, ended_time)

    Q_PROPERTY(bool completed READ isCompleted NOTIFY completedChanged)

public:
    explicit WorkoutModel(QObject *parent = nullptr);

    bool isCompleted() const;

    void addExercise(ExerciseModel *exercise);
    void removeExercise(ExerciseModel *exercise);

    QVariantMap toVariantMap(bool dbModel = false) const;
    static WorkoutModel *fromVariantMap(const QVariantMap &variantMap, QObject *parent = nullptr);

    QJsonObject toJson() const;
    static WorkoutModel *fromJson(const QJsonObject &jsonObj, QObject *parent = nullptr);

    WorkoutModel *clone(QObject *parent = nullptr) const;

signals:
    bool completedChanged();

private:
    void onIdUpdated();
};
