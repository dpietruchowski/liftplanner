#pragma once

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QQmlListProperty>
#include "modules/workout/domain/entities/workout.h"
#include "exercisemodel.h"

class WorkoutModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY dataChanged)
    Q_PROPERTY(QQmlListProperty<ExerciseModel> exercises READ exercisesProperty NOTIFY exercisesChanged)
    Q_PROPERTY(QDateTime createdTime READ createdTime NOTIFY dataChanged)
    Q_PROPERTY(QDateTime plannedTime READ plannedTime NOTIFY dataChanged)
    Q_PROPERTY(QDateTime startedTime READ startedTime NOTIFY dataChanged)
    Q_PROPERTY(QDateTime endedTime READ endedTime NOTIFY dataChanged)
    Q_PROPERTY(bool completed READ isCompleted NOTIFY completedChanged)

public:
    explicit WorkoutModel(QObject *parent = nullptr);
    explicit WorkoutModel(const Workout &workout, QObject *parent = nullptr);

    int id() const;
    QString name() const;
    QDateTime createdTime() const;
    QDateTime plannedTime() const;
    QDateTime startedTime() const;
    QDateTime endedTime() const;
    bool isCompleted() const;

    QQmlListProperty<ExerciseModel> exercisesProperty();
    QList<ExerciseModel *> exercises() const;

    void setStartedTime(const QDateTime &time);
    void setEndedTime(const QDateTime &time);

    void addExercise(ExerciseModel *exercise);

    Workout toEntity() const;
    WorkoutModel *clone(QObject *parent = nullptr) const;

signals:
    void dataChanged();
    void exercisesChanged();
    void completedChanged();

private:
    Workout m_workout;
    QList<ExerciseModel *> m_exercises;
};
