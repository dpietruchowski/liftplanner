#pragma once
#include <QObject>
#include <QVariantMap>
#include <QJsonObject>
#include <QList>
#include "setmodel.h"
#include "../utils/serializationutils.h"

class ExerciseModel : public QObject
{
    Q_OBJECT

    DECLARE_MODEL_PROPERTY(int, id, setId, id)
    DECLARE_MODEL_PROPERTY(int, workoutId, setWorkoutId, workout_id)
    DECLARE_MODEL_PROPERTY(QString, name, setName, name)
    DECLARE_MODEL_PROPERTY(QList<QObject*>, sets, setSets, sets)

public:
    explicit ExerciseModel(QObject *parent = nullptr);

    void addSet(SetModel *set);
    void removeSet(SetModel *set);

    QVariantMap toVariantMap(bool dbModel = false) const;
    static ExerciseModel* fromVariantMap(const QVariantMap &variantMap, QObject *parent = nullptr);

    QJsonObject toJson() const;
    static ExerciseModel* fromJson(const QJsonObject &jsonObj, QObject *parent = nullptr);

private:
    void onIdUpdated();
};
