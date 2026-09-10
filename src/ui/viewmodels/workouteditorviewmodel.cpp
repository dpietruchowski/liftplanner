#include "workouteditorviewmodel.h"

#include "application/workout/workoutservice.h"
#include "application/workout/workouttemplateservice.h"
#include "async/timeprovider.h"
#include "domain/workout/exerciseseeding.h"
#include "domain/workout/restadjustment.h"
#include "domain/workout/setadjustment.h"
#include "domain/workout/setcompatibility.h"
#include "domain/workout/setseeding.h"
#include "ui/presentation/workouttext.h"
#include <algorithm>

WorkoutEditorViewModel::WorkoutEditorViewModel(WorkoutService* service,
                                               WorkoutTemplateService* templateService,
                                               QObject* parent)
    : QObject(parent)
    , m_service(service)
    , m_templateService(templateService)
{
}

WorkoutEditorViewModel::~WorkoutEditorViewModel() = default;

WorkoutModel* WorkoutEditorViewModel::workout() const { return m_model; }

QString WorkoutEditorViewModel::name() const { return m_workout.name(); }

QDateTime WorkoutEditorViewModel::plannedTime() const { return m_workout.plannedTime(); }

QString WorkoutEditorViewModel::plannedDayText() const
{
    if (!m_editing)
        return QString();

    return WorkoutText::plannedDayLabel(m_workout.plannedTime().date(),
                                        TimeProvider::instance().currentDate());
}

void WorkoutEditorViewModel::shiftPlannedDay(int days)
{
    if (!m_editing || days == 0)
        return;

    const QDateTime current = m_workout.plannedTime().isValid()
        ? m_workout.plannedTime()
        : TimeProvider::instance().currentDateTime();

    setPlannedTime(current.addDays(days));
}

bool WorkoutEditorViewModel::isEditing() const { return m_editing; }

int WorkoutEditorViewModel::exerciseCount() const
{
    return static_cast<int>(m_workout.exercises().size());
}

bool WorkoutEditorViewModel::isValid() const { return m_editing && m_workout.isValid(); }

QStringList WorkoutEditorViewModel::validationErrors() const
{
    return m_editing ? m_workout.validationErrors() : QStringList();
}

bool WorkoutEditorViewModel::isDirty() const { return m_dirty; }

void WorkoutEditorViewModel::setName(const QString& value)
{
    if (!m_editing || m_workout.name() == value)
        return;

    m_workout.setName(value);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::setPlannedTime(const QDateTime& value)
{
    if (!m_editing || m_workout.plannedTime() == value)
        return;

    m_workout.setPlannedTime(value);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::createNew(const QString& name, const QDateTime& plannedTime)
{
    Workout created(name, TimeProvider::instance().currentDateTime());
    created.setPlannedTime(plannedTime);
    created.setStatus(WorkoutStatus::Planned);

    adopt(created, false);
}

void WorkoutEditorViewModel::edit(int workoutId)
{
    if (!m_service)
        return;

    m_service->findWorkout(workoutId)
        .then(this,
              [this, workoutId](std::optional<Workout> found)
              {
                  if (!found.has_value())
                  {
                      emit errorOccurred(
                          QStringLiteral("workout %1 does not exist").arg(workoutId));
                      return;
                  }

                  adopt(found.value(), false);
              })
        .onError(this, [this](const QString& error) { emit errorOccurred(error); });
}

void WorkoutEditorViewModel::startFromTemplate(int templateId, const QDateTime& plannedTime)
{
    if (!m_templateService)
        return;

    m_templateService->instantiate(templateId, plannedTime)
        .then(this, [this](Workout created) { adopt(created, true); })
        .onError(this, [this](const QString& error) { emit errorOccurred(error); });
}

void WorkoutEditorViewModel::discard()
{
    m_workout = Workout();
    m_editing = false;
    setDirty(false);
    publish();
}

void WorkoutEditorViewModel::addExercise(ExerciseDefinitionModel* definition)
{
    if (!m_editing || definition == nullptr)
        return;

    const ExerciseDefinition& entity = definition->entity();
    const Exercise exercise
        = seededExercise(entity.id(), entity.name(), entity.kind(), entity.defaultRestSeconds(),
                         entity.defaultMetric(), entity.defaultLoadType());

    m_workout.addExercise(exercise);
    markDirty();
    publish();

    seedFromHistory(static_cast<int>(m_workout.exercises().size()) - 1);
}

void WorkoutEditorViewModel::seedFromHistory(int exerciseIndex)
{
    const Exercise* exercise = exerciseAt(exerciseIndex);
    if (m_service == nullptr || exercise == nullptr || exercise->sets().size() != 1)
        return;

    const Exercise added = *exercise;
    const Set seed = added.sets().front();
    const QString expectedName = added.name();

    m_service->lastPerformance(added)
        .then(this,
              [this, exerciseIndex, expectedName, seed](std::optional<Exercise> previous)
              {
                  if (previous.has_value())
                      applyHistorySeed(exerciseIndex, expectedName, seed, previous.value());
              })
        .warnOnError("seed a new exercise from history");
}

void WorkoutEditorViewModel::applyHistorySeed(int exerciseIndex, const QString& expectedName,
                                              const Set& seed, const Exercise& previous)
{
    Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr || exercise->name() != expectedName || exercise->sets().size() != 1)
        return;

    Set& current = exercise->sets().front();
    if (current.completed() || !sameSetValues(current, seed))
        return;

    const std::optional<Set> seeded = seedFromPreviousExercise(previous, seed);
    if (!seeded.has_value())
        return;

    current = seeded.value();
    markDirty();
    publish();
}

void WorkoutEditorViewModel::removeExercise(int exerciseIndex)
{
    if (!m_editing || exerciseAt(exerciseIndex) == nullptr)
        return;

    m_workout.removeExercise(exerciseIndex);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::moveExercise(int from, int to)
{
    if (!m_editing || exerciseAt(from) == nullptr || exerciseAt(to) == nullptr || from == to)
        return;

    m_workout.moveExercise(from, to);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::setExerciseRest(int exerciseIndex, int restSeconds)
{
    Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr || restSeconds < 0 || exercise->restSeconds() == restSeconds)
        return;

    exercise->setRestSeconds(restSeconds);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::adjustExerciseRest(int exerciseIndex, int steps)
{
    const Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr || steps == 0)
        return;

    setExerciseRest(exerciseIndex, RestAdjustment::adjusted(exercise->restSeconds(), steps));
}

void WorkoutEditorViewModel::setExerciseNotes(int exerciseIndex, const QString& notes)
{
    Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr || exercise->notes() == notes)
        return;

    exercise->setNotes(notes);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::addSet(int exerciseIndex)
{
    Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr)
        return;

    if (exercise->sets().empty())
    {
        exercise->addSet(seedSetForKind(exercise->kind()));
    }
    else
    {
        Set copy = exercise->sets().back();
        copy.setId(-1);
        copy.setCompleted(false);
        exercise->addSet(copy);
    }

    markDirty();
    publish();
}

void WorkoutEditorViewModel::duplicateSet(int exerciseIndex, int setIndex)
{
    Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr || setAt(exerciseIndex, setIndex) == nullptr)
        return;

    exercise->duplicateSet(setIndex);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::removeSet(int exerciseIndex, int setIndex)
{
    Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr || setAt(exerciseIndex, setIndex) == nullptr)
        return;

    exercise->removeSet(setIndex);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::moveSet(int exerciseIndex, int from, int to)
{
    Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr || setAt(exerciseIndex, from) == nullptr
        || setAt(exerciseIndex, to) == nullptr || from == to)
        return;

    exercise->moveSet(from, to);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::setSetRepetitions(int exerciseIndex, int setIndex, int repetitions)
{
    Set* set = setAt(exerciseIndex, setIndex);
    if (set == nullptr || repetitions < 0 || set->repetitions() == repetitions)
        return;

    set->setRepetitions(repetitions);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::setSetWeight(int exerciseIndex, int setIndex, double weight)
{
    Set* set = setAt(exerciseIndex, setIndex);
    if (set == nullptr || qFuzzyCompare(set->weight(), weight))
        return;

    set->setWeight(weight);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::setSetDuration(int exerciseIndex, int setIndex, int seconds)
{
    Set* set = setAt(exerciseIndex, setIndex);
    if (set == nullptr || seconds < 0 || set->durationSeconds() == seconds)
        return;

    set->setDurationSeconds(seconds);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::setSetDistance(int exerciseIndex, int setIndex, double meters)
{
    Set* set = setAt(exerciseIndex, setIndex);
    if (set == nullptr || meters < 0.0 || qFuzzyCompare(set->distanceMeters(), meters))
        return;

    set->setDistanceMeters(meters);
    markDirty();
    publish();
}

void WorkoutEditorViewModel::adjustSetPrimary(int exerciseIndex, int setIndex, int steps)
{
    Set* set = setAt(exerciseIndex, setIndex);
    if (set == nullptr || steps == 0)
        return;

    switch (set->metric())
    {
        case SetMetric::Duration:
            set->setDurationSeconds(
                std::max(0, set->durationSeconds() + steps * SetAdjustment::durationSeconds));
            break;
        case SetMetric::Distance:
            set->setDistanceMeters(
                std::max(0.0, set->distanceMeters() + steps * SetAdjustment::distanceMeters));
            break;
        case SetMetric::Reps:
            set->setRepetitions(
                std::max(0, set->repetitions() + steps * SetAdjustment::repetitions));
            break;
    }

    markDirty();
    publish();
}

void WorkoutEditorViewModel::adjustSetSecondary(int exerciseIndex, int setIndex, int steps)
{
    Set* set = setAt(exerciseIndex, setIndex);
    if (set == nullptr || steps == 0)
        return;

    if (set->metric() == SetMetric::Distance)
        set->setDurationSeconds(
            std::max(0, set->durationSeconds() + steps * SetAdjustment::pacedDurationSeconds));
    else
        set->setWeight(std::max(0.0, set->weight() + steps * SetAdjustment::weightKilograms));

    markDirty();
    publish();
}

void WorkoutEditorViewModel::save()
{
    if (!m_service || !m_editing)
        return;

    const QStringList errors = m_workout.validationErrors();
    if (!errors.isEmpty())
    {
        emit errorOccurred(errors.join(QStringLiteral("; ")));
        return;
    }

    m_service->saveWorkout(m_workout)
        .then(this,
              [this](int savedId)
              {
                  m_workout.setId(savedId);
                  setDirty(false);
                  publish();
                  emit saved(savedId);
              })
        .onError(this, [this](const QString& error) { emit errorOccurred(error); });
}

void WorkoutEditorViewModel::saveAsTemplate(const QString& name)
{
    if (!m_templateService || !m_editing)
        return;

    m_templateService->saveFromWorkout(m_workout, name)
        .then(this, [this](int templateId) { emit savedAsTemplate(templateId); })
        .onError(this, [this](const QString& error) { emit errorOccurred(error); });
}

Exercise* WorkoutEditorViewModel::exerciseAt(int exerciseIndex)
{
    if (!m_editing || exerciseIndex < 0
        || exerciseIndex >= static_cast<int>(m_workout.exercises().size()))
        return nullptr;

    return &m_workout.exercises()[static_cast<size_t>(exerciseIndex)];
}

Set* WorkoutEditorViewModel::setAt(int exerciseIndex, int setIndex)
{
    Exercise* exercise = exerciseAt(exerciseIndex);
    if (exercise == nullptr || setIndex < 0
        || setIndex >= static_cast<int>(exercise->sets().size()))
        return nullptr;

    return &exercise->sets()[static_cast<size_t>(setIndex)];
}

void WorkoutEditorViewModel::adopt(const Workout& workout, bool dirty)
{
    m_workout = workout;
    m_editing = true;
    setDirty(dirty);
    publish();
}

void WorkoutEditorViewModel::publish()
{
    if (m_model != nullptr)
        m_model->deleteLater();

    m_model = m_editing ? new WorkoutModel(m_workout, this) : nullptr;

    emit workoutChanged();
}

void WorkoutEditorViewModel::markDirty() { setDirty(true); }

void WorkoutEditorViewModel::setDirty(bool value)
{
    if (m_dirty == value)
        return;

    m_dirty = value;
    emit dirtyChanged();
}
