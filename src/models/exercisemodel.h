#pragma once
#include <QJsonObject>
#include <QList>
#include <QObject>
#include "../utils/serializationutils.h"
#include "setmodel.h"
#include "types.h"

class ExerciseModel : public QObject
{
    Q_OBJECT

    DECLARE_MODEL_PROPERTY(int, id, setId, id)
    DECLARE_MODEL_PROPERTY(int, workoutId, setWorkoutId, workout_id)
    DECLARE_MODEL_PROPERTY(QString, name, setName, name)
    DECLARE_MODEL_PROPERTY(QString, description, setDescription, description)
    DECLARE_MODEL_PROPERTY(int, restSeconds, setRestSeconds, rest_seconds)
    DECLARE_MODEL_PROPERTY(QString, youtubeLink, setYoutubeLink, youtube_link)
    DECLARE_MODEL_LIST_PROPERTY(ExerciseModel, SetModel, sets, setSets, sets)

    Q_PROPERTY(bool completed READ isCompleted NOTIFY completedChanged)

public:
    explicit ExerciseModel(QObject *parent = nullptr);

    bool isCompleted() const;

    void addSet(SetModel *set);
    void removeSet(SetModel *set);

    QJsonObject toJson(SerializationMode mode) const;
    static ExerciseModel *fromJson(const QJsonObject &jsonObj, QObject *parent = nullptr);

    Q_INVOKABLE QString setsToString() const;
    void setsFromString(const QString &str);

    ExerciseModel *clone(QObject *parent = nullptr) const;

signals:
    bool completedChanged();
};
