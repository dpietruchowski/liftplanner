#include "activeworkoutviewmodel.h"
#include "application/workout/workoutservice.h"
#include "domain/workout/setadjustment.h"
#include "infrastructure/workout/workoutjson.h"
#include "platform/haptics.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QStandardPaths>
#include <algorithm>

namespace
{

QString cacheFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/current_workout.json");
}

std::optional<Workout> readCachedWorkout()
{
    QFile file(cacheFilePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return std::nullopt;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return std::nullopt;

    return WorkoutJson::workoutFromJson(doc.object());
}

Workout withExecutionStateFrom(Workout stored, const Workout& cached)
{
    QHash<int, bool> completedBySetId;
    for (const Exercise& exercise : cached.exercises())
    {
        for (const Set& set : exercise.sets())
        {
            if (set.id() != -1)
                completedBySetId.insert(set.id(), set.completed());
        }
    }

    for (Exercise& exercise : stored.exercises())
    {
        for (Set& set : exercise.sets())
        {
            const auto it = completedBySetId.constFind(set.id());
            if (it != completedBySetId.constEnd())
                set.setCompleted(it.value());
        }
    }

    stored.setStatus(cached.status());
    stored.setStartedTime(cached.startedTime());
    return stored;
}

QString completedSetsSummary(const Exercise& exercise)
{
    Exercise done = exercise;
    done.sets().clear();
    for (const Set& set : exercise.sets())
    {
        if (set.completed())
            done.addSet(set);
    }
    return done.setsToString();
}

}

ActiveWorkoutViewModel::ActiveWorkoutViewModel(WorkoutService* service, QObject* parent)
    : QObject(parent)
    , m_currentWorkout(nullptr)
    , m_currentExercise(nullptr)
    , m_currentSet(nullptr)
    , m_isActive(false)
    , m_service(service)
    , m_timer(new WorkoutTimer(this))
{
    connect(m_timer, &WorkoutTimer::finished, this, &ActiveWorkoutViewModel::onTimerFinished);
    connect(this, &ActiveWorkoutViewModel::currentWorkoutChanged, this,
            &ActiveWorkoutViewModel::saveCurrentWorkout);
    connect(this, &ActiveWorkoutViewModel::currentExerciseChanged, this,
            &ActiveWorkoutViewModel::saveCurrentWorkout);
    connect(this, &ActiveWorkoutViewModel::currentSetChanged, this,
            &ActiveWorkoutViewModel::saveCurrentWorkout);
    loadCurrentWorkout();
}

ActiveWorkoutViewModel::~ActiveWorkoutViewModel()
{
    m_shuttingDown = true;
    saveCurrentWorkout();
}

WorkoutTimer* ActiveWorkoutViewModel::timer() const { return m_timer; }

void ActiveWorkoutViewModel::cacheCurrentWorkout()
{
    const QString filePath = cacheFilePath();

    if (!m_currentWorkout)
    {
        QFile::remove(filePath);
        return;
    }

    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
        file.write(
            QJsonDocument(WorkoutJson::workoutToJson(m_currentWorkout->toEntity())).toJson());
}

void ActiveWorkoutViewModel::saveCurrentWorkout()
{
    cacheCurrentWorkout();
    saveToDb();
}

void ActiveWorkoutViewModel::loadCurrentWorkout()
{
    const std::optional<Workout> cached = readCachedWorkout();
    if (!cached)
        return;

    if (cached->id() == -1 || !m_service)
    {
        restoreWorkout(*cached);
        return;
    }

    m_service->findWorkout(cached->id())
        .then(this, [this, cached = *cached](std::optional<Workout> stored)
              { restoreWorkout(stored ? withExecutionStateFrom(*stored, cached) : cached); })
        .onError(this, [this, cached = *cached](const QString&) { restoreWorkout(cached); });
}

void ActiveWorkoutViewModel::restoreWorkout(const Workout& entity)
{
    if (m_currentWorkout)
        return;

    setCurrentWorkout(new WorkoutModel(entity, this));
    selectFirstIncomplete();
    setIsActive(true);
    refreshPreviousPerformances();
}

void ActiveWorkoutViewModel::refreshPreviousPerformances()
{
    if (!m_service || !m_currentWorkout)
        return;

    QPointer<WorkoutModel> target(m_currentWorkout);
    m_service->previousPerformances(m_currentWorkout->toEntity())
        .then(this,
              [target](std::vector<WorkoutService::PreviousPerformance> entries)
              {
                  if (!target)
                      return;

                  const QList<ExerciseModel*> exercises = target->exercises();
                  for (const auto& entry : entries)
                  {
                      if (entry.exerciseIndex < 0 || entry.exerciseIndex >= exercises.size())
                          continue;
                      exercises[entry.exerciseIndex]->setPreviousPerformance(
                          completedSetsSummary(entry.exercise), entry.performedAt);
                  }
              })
        .warnOnError("load previous performances");
}

void ActiveWorkoutViewModel::startWorkout(WorkoutModel* workout)
{
    if (!workout)
    {
        emit errorOccurred("Invalid workout");
        return;
    }

    auto* previousWorkout = m_currentWorkout;

    m_timer->stop();

    auto* clonedWorkout = workout->clone(nullptr);
    clonedWorkout->start();

    setCurrentWorkout(clonedWorkout);
    setIsActive(true);

    updateCurrentExercise();
    updateCurrentSet();
    refreshPreviousPerformances();

    if (previousWorkout)
    {
        if (m_service && previousWorkout->id() != -1)
            m_service->deleteWorkout(previousWorkout->id())
                .warnOnError("delete the replaced workout");
        previousWorkout->deleteLater();
    }

    qDebug() << "Workout started:" << clonedWorkout->name();
}

void ActiveWorkoutViewModel::completeCurrentSet()
{
    if (!m_isActive || !m_currentSet)
    {
        emit errorOccurred("No active set");
        return;
    }

    const int rest = restSecondsFor(m_currentSet);

    saveCompletedSet();
    selectNextIncomplete();
    saveCurrentWorkout();

    startRestAfterCompletedSet(rest);
}

void ActiveWorkoutViewModel::navigateToNext()
{
    if (!m_isActive || !m_currentWorkout || !m_currentExercise)
        return;

    auto sets = m_currentExercise->sets();
    int currentSetIndex = sets.indexOf(m_currentSet);

    if (currentSetIndex < sets.size() - 1)
    {
        setCurrentSet(sets[currentSetIndex + 1]);
    }
    else
    {
        auto exercises = m_currentWorkout->exercises();
        int currentExerciseIndex = exercises.indexOf(m_currentExercise);

        if (currentExerciseIndex < exercises.size() - 1)
        {
            setCurrentExercise(exercises[currentExerciseIndex + 1]);

            auto newSets = m_currentExercise->sets();
            setCurrentSet(newSets.isEmpty() ? nullptr : newSets[0]);
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
        setCurrentSet(sets[currentSetIndex - 1]);
    }
    else
    {
        auto exercises = m_currentWorkout->exercises();
        int currentExerciseIndex = exercises.indexOf(m_currentExercise);

        if (currentExerciseIndex > 0)
        {
            setCurrentExercise(exercises[currentExerciseIndex - 1]);

            auto prevSets = m_currentExercise->sets();
            setCurrentSet(prevSets.isEmpty() ? nullptr : prevSets.last());
        }
    }
}

void ActiveWorkoutViewModel::endWorkout()
{
    if (!m_currentWorkout)
        return;

    m_timer->stop();
    m_currentWorkout->end();
    saveToDb();
    emit workoutCompleted();

    auto* oldWorkout = m_currentWorkout;

    setIsActive(false);
    setCurrentWorkout(nullptr);
    setCurrentExercise(nullptr);
    setCurrentSet(nullptr);

    oldWorkout->deleteLater();
}

void ActiveWorkoutViewModel::duplicateSet(SetModel* set)
{
    if (!set)
        return;

    auto* exercise = qobject_cast<ExerciseModel*>(set->parent());
    if (!exercise)
        return;

    Set duplicate = set->entity();
    duplicate.setId(-1);
    duplicate.setExerciseId(-1);
    duplicate.setCompleted(false);

    auto* clone = new SetModel(duplicate, exercise);
    exercise->addSet(clone);
    selectFirstIncomplete();
    saveCurrentWorkout();
}

void ActiveWorkoutViewModel::removeSet(SetModel* set)
{
    if (!set)
        return;

    auto* exercise = qobject_cast<ExerciseModel*>(set->parent());
    if (!exercise)
        return;

    auto sets = exercise->sets();
    int index = sets.indexOf(set);

    if (index < 0)
        return;

    exercise->removeSet(set);
    set->deleteLater();

    selectFirstIncomplete();
    saveCurrentWorkout();
}

void ActiveWorkoutViewModel::toggleSetCompleted(SetModel* set)
{
    if (!m_isActive || !set)
        return;

    if (!qobject_cast<ExerciseModel*>(set->parent()))
        return;

    set->setCompleted(!set->completed());

    selectFirstIncomplete();
    saveCurrentWorkout();
}

void ActiveWorkoutViewModel::adjustSetPrimary(SetModel* set, int steps)
{
    if (!set || steps == 0)
        return;

    const Set& entity = set->entity();
    switch (entity.metric())
    {
        case SetMetric::Duration:
            set->setDurationSeconds(
                std::max(0, entity.durationSeconds() + steps * SetAdjustment::durationSeconds));
            break;
        case SetMetric::Distance:
            set->setDistanceMeters(
                std::max(0.0, entity.distanceMeters() + steps * SetAdjustment::distanceMeters));
            break;
        case SetMetric::Reps:
            set->setRepetitions(
                std::max(0, entity.repetitions() + steps * SetAdjustment::repetitions));
            break;
    }

    saveSetToDb(set);
}

void ActiveWorkoutViewModel::adjustSetSecondary(SetModel* set, int steps)
{
    if (!set || steps == 0 || !set->secondaryAdjustable())
        return;

    const Set& entity = set->entity();
    if (entity.metric() == SetMetric::Distance)
        set->setDurationSeconds(
            std::max(0, entity.durationSeconds() + steps * SetAdjustment::pacedDurationSeconds));
    else
        set->setWeight(std::max(0.0, entity.weight() + steps * SetAdjustment::weightKilograms));

    saveSetToDb(set);
}

void ActiveWorkoutViewModel::moveExercise(int from, int to)
{
    if (!m_currentWorkout)
        return;

    m_currentWorkout->moveExercise(from, to);
    selectFirstIncomplete();
    saveCurrentWorkout();
}

void ActiveWorkoutViewModel::startWorkTimer()
{
    const int seconds = workSecondsFor(m_currentSet);
    if (seconds > 0)
        m_timer->start(WorkoutTimer::Work, seconds);
}

void ActiveWorkoutViewModel::startRestTimer()
{
    constexpr int fallback_rest_seconds = 60;

    const int rest = restSecondsFor(m_currentSet);
    m_timer->start(WorkoutTimer::Rest, rest > 0 ? rest : fallback_rest_seconds);
}

void ActiveWorkoutViewModel::toggleTimer()
{
    if (!m_isActive)
        return;

    if (m_timer->isRunning())
    {
        m_timer->stop();
        return;
    }

    if (workSecondsFor(m_currentSet) > 0)
        startWorkTimer();
    else
        startRestTimer();
}

void ActiveWorkoutViewModel::onTimerFinished(WorkoutTimer::Phase phase)
{
    if (phase == WorkoutTimer::Work)
    {
        Haptics::play(Haptics::Effect::LevelUp);
        completeCurrentSet();
        return;
    }

    Haptics::play(Haptics::Effect::Reward);
    startWorkForCurrentSet();
}

void ActiveWorkoutViewModel::startRestAfterCompletedSet(int restSeconds)
{
    if (m_currentWorkout && m_currentWorkout->isCompleted())
    {
        m_timer->stop();
        return;
    }

    if (restSeconds > 0)
        m_timer->start(WorkoutTimer::Rest, restSeconds);
    else
        startWorkForCurrentSet();
}

void ActiveWorkoutViewModel::startWorkForCurrentSet()
{
    const int seconds = workSecondsFor(m_currentSet);
    if (seconds > 0)
        m_timer->start(WorkoutTimer::Work, seconds);
    else
        m_timer->stop();
}

int ActiveWorkoutViewModel::restSecondsFor(SetModel* set) const
{
    if (!set)
        return 0;

    auto* exercise = qobject_cast<ExerciseModel*>(set->parent());
    if (!exercise)
        return 0;

    return set->entity().effectiveRestSeconds(exercise->restSeconds());
}

int ActiveWorkoutViewModel::workSecondsFor(SetModel* set) const
{
    if (!set || set->entity().metric() != SetMetric::Duration)
        return 0;

    return set->entity().durationSeconds();
}

void ActiveWorkoutViewModel::saveCompletedSet()
{
    if (!m_currentSet)
        return;

    m_currentSet->setCompleted(true);

    qDebug() << "Completed set:" << (m_currentExercise ? m_currentExercise->name() : "Unknown")
             << m_currentSet->repetitions() << "reps x" << m_currentSet->weight() << "kg";
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

    setCurrentSet(m_currentExercise->sets().first());
}

void ActiveWorkoutViewModel::selectFirstIncomplete()
{
    if (!m_currentWorkout || m_currentWorkout->exercises().isEmpty())
    {
        setCurrentExercise(nullptr);
        setCurrentSet(nullptr);
        return;
    }

    for (auto* exercise : m_currentWorkout->exercises())
    {
        for (auto* set : exercise->sets())
        {
            if (!set->completed())
            {
                setCurrentExercise(exercise);
                setCurrentSet(set);
                return;
            }
        }
    }

    updateCurrentExercise();
    updateCurrentSet();
}

void ActiveWorkoutViewModel::selectNextIncomplete()
{
    if (!m_currentWorkout || !m_currentExercise)
        return;

    auto exercises = m_currentWorkout->exercises();
    int exIdx = exercises.indexOf(m_currentExercise);
    int setIdx = m_currentExercise->sets().indexOf(m_currentSet);

    for (int e = exIdx; e >= 0 && e < exercises.size(); ++e)
    {
        auto sets = exercises[e]->sets();
        int start = (e == exIdx) ? setIdx + 1 : 0;
        for (int s = start; s < sets.size(); ++s)
        {
            if (!sets[s]->completed())
            {
                setCurrentExercise(exercises[e]);
                setCurrentSet(sets[s]);
                return;
            }
        }
    }

    selectFirstIncomplete();
}

void ActiveWorkoutViewModel::saveToDb()
{
    if (!m_service || !m_currentWorkout)
        return;

    if (m_shuttingDown)
    {
        m_service->saveWorkout(m_currentWorkout->toEntity()).warnOnError("save the active workout");
        return;
    }

    QPointer<WorkoutModel> target(m_currentWorkout);
    m_service->saveWorkout(m_currentWorkout->toEntity())
        .then(this,
              [target](int savedId)
              {
                  if (target)
                      target->setId(savedId);
              })
        .warnOnError("save the active workout");
}

void ActiveWorkoutViewModel::saveSetToDb(SetModel* set)
{
    cacheCurrentWorkout();

    if (!m_service || !set || set->entity().id() == -1)
    {
        saveToDb();
        return;
    }

    m_service->saveSet(set->entity()).warnOnError("save the set");
}
