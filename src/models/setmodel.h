#pragma once
#include <QObject>
#include <QVariantMap>
#include <QJsonObject>
#include "../utils/serializationutils.h"

class SetModel : public QObject
{
    Q_OBJECT

    DECLARE_MODEL_PROPERTY(int, id, setId, id)
    DECLARE_MODEL_PROPERTY(int, exerciseId, setExerciseId, exercise_id)
    DECLARE_MODEL_PROPERTY(int, repetitions, setRepetitions, repetitions)
    DECLARE_MODEL_PROPERTY(int, weight, setWeight, weight)
    DECLARE_MODEL_PROPERTY(bool, completed, setCompleted, completed)

public:
    explicit SetModel(QObject *parent = nullptr);

    QVariantMap toVariantMap(bool dbModel = false) const;
    static SetModel *fromVariantMap(const QVariantMap &variantMap, QObject *parent = nullptr);

    QJsonObject toJson() const;
    static SetModel *fromJson(const QJsonObject &jsonObj, QObject *parent = nullptr);

    QString toString() const;
    static SetModel *fromString(const QString &str, QObject *parent = nullptr);

    SetModel *clone(QObject *parent = nullptr) const;
};
