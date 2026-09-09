#pragma once

#include "domain/workout/workout.h"
#include "domain/workout/workoutstatus.h"
#include "exercisemodel.h"
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QQmlListProperty>

class WorkoutModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id NOTIFY dataChanged)
    Q_PROPERTY(QString name READ name NOTIFY dataChanged)
    Q_PROPERTY(
        QQmlListProperty<ExerciseModel> exercises READ exercisesProperty NOTIFY exercisesChanged)
    Q_PROPERTY(QDateTime createdTime READ createdTime NOTIFY dataChanged)
    Q_PROPERTY(QDateTime plannedTime READ plannedTime NOTIFY dataChanged)
    Q_PROPERTY(QDateTime startedTime READ startedTime NOTIFY dataChanged)
    Q_PROPERTY(QDateTime endedTime READ endedTime NOTIFY dataChanged)
    Q_PROPERTY(bool completed READ isCompleted NOTIFY completedChanged)
    Q_PROPERTY(QString status READ statusString NOTIFY dataChanged)

public:
    explicit WorkoutModel(QObject* parent = nullptr);
    explicit WorkoutModel(const Workout& workout, QObject* parent = nullptr);

    int id() const;
    QString name() const;
    QDateTime createdTime() const;
    QDateTime plannedTime() const;
    QDateTime startedTime() const;
    QDateTime endedTime() const;
    bool isCompleted() const;
    WorkoutStatus status() const;
    QString statusString() const;

    QQmlListProperty<ExerciseModel> exercisesProperty();
    QList<ExerciseModel*> exercises() const;

    void setId(int id);
    void setStartedTime(const QDateTime& time);
    void setEndedTime(const QDateTime& time);
    void start();
    void end();

    void addExercise(ExerciseModel* exercise);
    void moveExercise(int from, int to);

    Workout toEntity() const;
    WorkoutModel* clone(QObject* parent = nullptr) const;

signals:
    void dataChanged();
    void exercisesChanged();
    void completedChanged();

private:
    Workout m_record;
    QList<ExerciseModel*> m_exercises;
};
