#include "workoutmodel.h"

WorkoutModel::WorkoutModel(QObject *parent)
    : QObject(parent)
{
}

WorkoutModel::WorkoutModel(const Workout &workout, QObject *parent)
    : QObject(parent), m_workout(workout)
{
    for (const auto &exercise : workout.exercises())
    {
        auto *exerciseModel = new ExerciseModel(exercise, this);
        connect(exerciseModel, &ExerciseModel::completedChanged, this, &WorkoutModel::completedChanged);
        m_exercises.append(exerciseModel);
    }
}

int WorkoutModel::id() const { return m_workout.id(); }
QString WorkoutModel::name() const { return m_workout.name(); }
QDateTime WorkoutModel::createdTime() const { return m_workout.createdTime(); }
QDateTime WorkoutModel::plannedTime() const { return m_workout.plannedTime(); }
QDateTime WorkoutModel::startedTime() const { return m_workout.startedTime(); }
QDateTime WorkoutModel::endedTime() const { return m_workout.endedTime(); }

bool WorkoutModel::isCompleted() const
{
    if (m_exercises.isEmpty())
        return false;
    for (auto *e : m_exercises)
    {
        if (!e->isCompleted())
            return false;
    }
    return true;
}

QQmlListProperty<ExerciseModel> WorkoutModel::exercisesProperty()
{
    return QQmlListProperty<ExerciseModel>(this, &m_exercises);
}

QList<ExerciseModel *> WorkoutModel::exercises() const
{
    return m_exercises;
}

void WorkoutModel::setStartedTime(const QDateTime &time)
{
    m_workout.setStartedTime(time);
    emit dataChanged();
}

void WorkoutModel::setEndedTime(const QDateTime &time)
{
    m_workout.setEndedTime(time);
    emit dataChanged();
}

void WorkoutModel::addExercise(ExerciseModel *exercise)
{
    if (exercise && !m_exercises.contains(exercise))
    {
        exercise->setParent(this);
        connect(exercise, &ExerciseModel::completedChanged, this, &WorkoutModel::completedChanged);
        m_exercises.append(exercise);
        emit exercisesChanged();
        emit completedChanged();
    }
}

Workout WorkoutModel::toEntity() const
{
    Workout w;
    w.setId(m_workout.id());
    w.setName(m_workout.name());
    w.setCreatedTime(m_workout.createdTime());
    w.setPlannedTime(m_workout.plannedTime());
    w.setStartedTime(m_workout.startedTime());
    w.setEndedTime(m_workout.endedTime());
    for (auto *e : m_exercises)
        w.addExercise(e->toEntity());
    return w;
}

WorkoutModel *WorkoutModel::clone(QObject *parent) const
{
    return new WorkoutModel(toEntity(), parent);
}
