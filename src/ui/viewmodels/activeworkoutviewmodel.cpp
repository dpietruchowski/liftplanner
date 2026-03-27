#include "activeworkoutviewmodel.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

ActiveWorkoutViewModel::ActiveWorkoutViewModel(QObject *parent)
    : QObject(parent), m_currentWorkout(nullptr), m_currentExercise(nullptr), m_currentSet(nullptr), m_isActive(false)
{
    connect(this,
            &ActiveWorkoutViewModel::currentWorkoutChanged,
            this,
            &ActiveWorkoutViewModel::saveCurrentWorkout);
    connect(this,
            &ActiveWorkoutViewModel::currentExerciseChanged,
            this,
            &ActiveWorkoutViewModel::saveCurrentWorkout);
    connect(this,
            &ActiveWorkoutViewModel::currentSetChanged,
            this,
            &ActiveWorkoutViewModel::saveCurrentWorkout);
    loadCurrentWorkout();
}

ActiveWorkoutViewModel::~ActiveWorkoutViewModel()
{
    saveCurrentWorkout();
}

void ActiveWorkoutViewModel::saveCurrentWorkout()
{
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dirPath);
    if (!dir.exists())
        dir.mkpath(".");

    QString filePath = dir.filePath("current_workout.json");

    if (!m_currentWorkout)
    {
        QFile::remove(filePath);
        return;
    }

    QJsonObject json = m_currentWorkout->toJson(SerializationMode::FullFile);
    QJsonDocument doc(json);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(doc.toJson());
        file.close();
    }
}

void ActiveWorkoutViewModel::loadCurrentWorkout()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/current_workout.json";

    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return;

    WorkoutModel *loadedWorkout = WorkoutModel::fromJson(doc.object(), this);
    if (loadedWorkout)
    {
        setCurrentWorkout(loadedWorkout);
        updateCurrentExercise();
        updateCurrentSet();
        setIsActive(true);
    }
}

void ActiveWorkoutViewModel::startWorkout(WorkoutModel *workout)
{
    if (!workout)
    {
        emit errorOccurred("Invalid workout");
        return;
    }

    auto *clonedWorkout = workout->clone(nullptr);
    clonedWorkout->setStartedTime(QDateTime::currentDateTime());

    setCurrentWorkout(clonedWorkout);
    setIsActive(true);

    updateCurrentExercise();
    updateCurrentSet();

    qDebug() << "Workout started:" << clonedWorkout->name();
}

void ActiveWorkoutViewModel::completeCurrentSet()
{
    if (!m_isActive || !m_currentSet)
    {
        emit errorOccurred("No active set");
        return;
    }

    saveCompletedSet();
    navigateToNext();
    saveCurrentWorkout();
}

void ActiveWorkoutViewModel::navigateToNext()
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
            setCurrentExercise(exercises[currentExerciseIndex + 1]);

            auto newSets = m_currentExercise->sets();
            setCurrentSet(newSets.isEmpty() ? nullptr : qobject_cast<SetModel *>(newSets[0]));
        }
    }
}

void ActiveWorkoutViewModel::navigateToPrevious()
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
            setCurrentExercise(exercises[currentExerciseIndex - 1]);

            auto prevSets = m_currentExercise->sets();
            setCurrentSet(prevSets.isEmpty() ? nullptr : qobject_cast<SetModel *>(prevSets.last()));
        }
    }
}

void ActiveWorkoutViewModel::endWorkout()
{
    if (m_currentWorkout)
    {
        m_currentWorkout->setEndedTime(QDateTime::currentDateTime());
        emit workoutCompleted();
    }

    setIsActive(false);
    setCurrentWorkout(nullptr);
    setCurrentExercise(nullptr);
    setCurrentSet(nullptr);

    if (m_currentWorkout)
    {
        m_currentWorkout->deleteLater();
    }
}

void ActiveWorkoutViewModel::duplicateSet(SetModel *set)
{
    if (!set)
        return;

    auto *exercise = qobject_cast<ExerciseModel *>(set->parent());
    if (!exercise)
        return;

    SetModel *clone = set->clone(exercise);
    if (!clone)
        return;

    clone->setCompleted(false);

    exercise->addSet(clone);
    setCurrentSet(clone);
    saveCurrentWorkout();
}

void ActiveWorkoutViewModel::removeSet(SetModel *set)
{
    if (!set)
        return;

    auto *exercise = qobject_cast<ExerciseModel *>(set->parent());
    if (!exercise)
        return;

    auto sets = exercise->sets();
    int index = sets.indexOf(set);

    if (index < 0)
        return;

    exercise->removeSet(set);
    set->deleteLater();

    if (m_currentSet == set)
    {
        if (!exercise->sets().isEmpty())
        {
            int newIndex = qMax(0, index - 1);
            setCurrentSet(qobject_cast<SetModel *>(exercise->sets().at(newIndex)));
        }
        else
        {
            setCurrentSet(nullptr);
        }
    }
    saveCurrentWorkout();
}

void ActiveWorkoutViewModel::saveCompletedSet()
{
    if (!m_currentSet)
        return;

    m_currentSet->setCompleted(true);

    qDebug() << "Completed set:"
             << (m_currentExercise ? m_currentExercise->name() : "Unknown")
             << m_currentSet->repetitions() << "reps x"
             << m_currentSet->weight() << "kg";
}

void ActiveWorkoutViewModel::updateCurrentExercise()
{
    if (!m_currentWorkout || m_currentWorkout->exercises().isEmpty())
    {
        setCurrentExercise(nullptr);
        return;
    }

    setCurrentExercise(m_currentWorkout->exercises().first());
}

void ActiveWorkoutViewModel::updateCurrentSet()
{
    if (!m_currentExercise || m_currentExercise->sets().isEmpty())
    {
        setCurrentSet(nullptr);
        return;
    }

    setCurrentSet(qobject_cast<SetModel *>(m_currentExercise->sets().first()));
}
