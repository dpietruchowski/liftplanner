#pragma once

#include <QObject>
#include <QList>
#include <QQmlListProperty>
#include "modules/workout/domain/entities/exercise.h"
#include "setmodel.h"

class ExerciseModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY dataChanged)
    Q_PROPERTY(QString description READ description NOTIFY dataChanged)
    Q_PROPERTY(int restSeconds READ restSeconds NOTIFY dataChanged)
    Q_PROPERTY(QString youtubeLink READ youtubeLink NOTIFY dataChanged)
    Q_PROPERTY(QQmlListProperty<SetModel> sets READ setsProperty NOTIFY setsChanged)
    Q_PROPERTY(bool completed READ isCompleted NOTIFY completedChanged)

public:
    explicit ExerciseModel(QObject *parent = nullptr);
    explicit ExerciseModel(const Exercise &exercise, QObject *parent = nullptr);

    QString name() const;
    QString description() const;
    int restSeconds() const;
    QString youtubeLink() const;
    bool isCompleted() const;

    QQmlListProperty<SetModel> setsProperty();
    QList<SetModel *> sets() const;

    Q_INVOKABLE QString setsToString() const;

    void addSet(SetModel *set);
    void removeSet(SetModel *set);

    Exercise toEntity() const;
    ExerciseModel *clone(QObject *parent = nullptr) const;

signals:
    void dataChanged();
    void setsChanged();
    void completedChanged();

private:
    Exercise m_exercise;
    QList<SetModel *> m_sets;
};
