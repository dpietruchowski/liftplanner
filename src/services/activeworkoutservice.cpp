#include "activeworkoutservice.h"
#include <QDebug>

ActiveWorkoutService::ActiveWorkoutService(QObject *parent)
    : QObject(parent), m_currentWorkout(nullptr), m_currentExercise(nullptr), m_currentSet(nullptr), m_isActive(false)
{
}

void ActiveWorkoutService::startWorkout(WorkoutModel *workout)
{
    if (!workout)
    {
        emit errorOccurred("Invalid workout");
        return;
    }

    auto *clonedWorkout = workout->clone(this);
    clonedWorkout->setStartedTime(QDateTime::currentDateTime());

    setCurrentWorkout(clonedWorkout);
    setIsActive(true);

    updateCurrentExercise();
    updateCurrentSet();

    qDebug() << "Workout started:" << clonedWorkout->name();
}

void ActiveWorkoutService::completeCurrentSet()
{
    if (!m_isActive || !m_currentSet)
    {
        emit errorOccurred("No active set");
        return;
    }

    saveCompletedSet();
    navigateToNext();
}

void ActiveWorkoutService::navigateToNext()
{
    if (!m_isActive || !m_currentWorkout || !m_currentExercise)
        return;

    auto sets = m_currentExercise->sets();
    int currentSetIndex = sets.indexOf(m_currentSet);

    if (currentSetIndex < sets.size() - 1)
    {
        setCurrentSet(qobject_cast<SetModel *>(sets[currentSetIndex + 1]));
    }
    else
    {
        auto exercises = m_currentWorkout->exercises();
        int currentExerciseIndex = exercises.indexOf(m_currentExercise);

        if (currentExerciseIndex < exercises.size() - 1)
        {
            setCurrentExercise(qobject_cast<ExerciseModel *>(exercises[currentExerciseIndex + 1]));

            auto newSets = m_currentExercise->sets();
            setCurrentSet(newSets.isEmpty() ? nullptr : qobject_cast<SetModel *>(newSets[0]));
        }
        else
        {
            endWorkout();
            emit workoutCompleted();
        }
    }
}

void ActiveWorkoutService::navigateToPrevious()
{
    if (!m_isActive || !m_currentWorkout || !m_currentExercise || !m_currentSet)
        return;

    auto sets = m_currentExercise->sets();
    int currentSetIndex = sets.indexOf(m_currentSet);

    if (currentSetIndex > 0)
    {
        setCurrentSet(qobject_cast<SetModel *>(sets[currentSetIndex - 1]));
    }
    else
    {
        auto exercises = m_currentWorkout->exercises();
        int currentExerciseIndex = exercises.indexOf(m_currentExercise);

        if (currentExerciseIndex > 0)
        {
            setCurrentExercise(qobject_cast<ExerciseModel *>(exercises[currentExerciseIndex - 1]));

            auto prevSets = m_currentExercise->sets();
            setCurrentSet(prevSets.isEmpty() ? nullptr : qobject_cast<SetModel *>(prevSets.last()));
        }
    }
}

void ActiveWorkoutService::endWorkout()
{
    if (m_currentWorkout) {
        m_currentWorkout->setEndedTime(QDateTime::currentDateTime());
    }

    setIsActive(false);
    setCurrentWorkout(nullptr);
    setCurrentExercise(nullptr);
    setCurrentSet(nullptr);

    qDebug() << "Workout ended";
}

void ActiveWorkoutService::saveCompletedSet()
{
    if (!m_currentSet)
        return;

    m_currentSet->setCompleted(true);

    qDebug() << "Completed set:"
             << (m_currentExercise ? m_currentExercise->name() : "Unknown")
             << m_currentSet->repetitions() << "reps x"
             << m_currentSet->weight() << "kg";
}

void ActiveWorkoutService::updateCurrentExercise()
{
    if (!m_currentWorkout || m_currentWorkout->exercises().isEmpty())
    {
        setCurrentExercise(nullptr);
        return;
    }

    setCurrentExercise(qobject_cast<ExerciseModel *>(m_currentWorkout->exercises().first()));
}

void ActiveWorkoutService::updateCurrentSet()
{
    if (!m_currentExercise || m_currentExercise->sets().isEmpty())
    {
        setCurrentSet(nullptr);
        return;
    }

    setCurrentSet(qobject_cast<SetModel *>(m_currentExercise->sets().first()));
}
