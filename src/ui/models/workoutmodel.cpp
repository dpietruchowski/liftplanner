#include "workoutmodel.h"

namespace
{
Workout recordOf(const Workout& workout)
{
    Workout record = workout;
    record.exercises().clear();
    return record;
}
}

WorkoutModel::WorkoutModel(QObject* parent)
    : QObject(parent)
{
}

WorkoutModel::WorkoutModel(const Workout& workout, QObject* parent)
    : QObject(parent)
    , m_record(recordOf(workout))
{
    for (const auto& exercise : workout.exercises())
    {
        auto* exerciseModel = new ExerciseModel(exercise, this);
        connect(exerciseModel, &ExerciseModel::completedChanged, this,
                &WorkoutModel::completedChanged);
        m_exercises.append(exerciseModel);
    }
}

int WorkoutModel::id() const { return m_record.id(); }
QString WorkoutModel::name() const { return m_record.name(); }
QDateTime WorkoutModel::createdTime() const { return m_record.createdTime(); }
QDateTime WorkoutModel::plannedTime() const { return m_record.plannedTime(); }
QDateTime WorkoutModel::startedTime() const { return m_record.startedTime(); }
QDateTime WorkoutModel::endedTime() const { return m_record.endedTime(); }

bool WorkoutModel::isCompleted() const { return toEntity().isCompleted(); }

WorkoutStatus WorkoutModel::status() const { return m_record.status(); }
QString WorkoutModel::statusString() const { return workoutStatusToString(m_record.status()); }

QQmlListProperty<ExerciseModel> WorkoutModel::exercisesProperty()
{
    return QQmlListProperty<ExerciseModel>(this, &m_exercises);
}

QList<ExerciseModel*> WorkoutModel::exercises() const { return m_exercises; }

void WorkoutModel::setId(int id)
{
    if (m_record.id() == id)
        return;

    m_record.setId(id);
    emit dataChanged();
}

void WorkoutModel::setStartedTime(const QDateTime& time)
{
    m_record.setStartedTime(time);
    emit dataChanged();
}

void WorkoutModel::setEndedTime(const QDateTime& time)
{
    m_record.setEndedTime(time);
    emit dataChanged();
}

void WorkoutModel::start()
{
    m_record.start();
    emit dataChanged();
}

void WorkoutModel::end()
{
    m_record.end();
    emit dataChanged();
}

void WorkoutModel::addExercise(ExerciseModel* exercise)
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

void WorkoutModel::moveExercise(int from, int to)
{
    if (from < 0 || from >= m_exercises.size() || to < 0 || to >= m_exercises.size() || from == to)
        return;

    m_exercises.move(from, to);
    emit exercisesChanged();
}

Workout WorkoutModel::toEntity() const
{
    Workout workout = m_record;
    for (auto* exercise : m_exercises)
        workout.addExercise(exercise->toEntity());
    return workout;
}

WorkoutModel* WorkoutModel::clone(QObject* parent) const
{
    return new WorkoutModel(toEntity(), parent);
}
