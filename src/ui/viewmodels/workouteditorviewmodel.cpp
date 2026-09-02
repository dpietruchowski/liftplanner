#include "workouteditorviewmodel.h"

#include "modules/workout/application/workoutservice.h"
#include "modules/workout/application/workouttemplateservice.h"
#include "modules/workout/domain/entities/setadjustment.h"
#include "modules/workout/domain/entities/setcompatibility.h"
#include "utils/timeprovider.h"
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
        .onError(this, [this](const QString& error) { emit errorOccurred(error); })
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
              });
}

void WorkoutEditorViewModel::startFromTemplate(int templateId, const QDateTime& plannedTime)
{
    if (!m_templateService)
        return;

    m_templateService->instantiate(templateId, plannedTime)
        .onError(this, [this](const QString& error) { emit errorOccurred(error); })
        .then(this, [this](Workout created) { adopt(created, true); });
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
    Exercise exercise = Exercise::createFromDefinition(entity.id(), entity.name(), entity.kind(),
                                                       entity.defaultRestSeconds());
    exercise.addSet(seedSetFor(*definition));

    m_workout.addExercise(exercise);
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
    if (exercise == nullptr || exercise->sets().empty())
        return;

    Set copy = exercise->sets().back();
    copy.setId(-1);
    copy.setCompleted(false);
    exercise->addSet(copy);

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
        .onError(this, [this](const QString& error) { emit errorOccurred(error); })
        .then(this,
              [this](int savedId)
              {
                  m_workout.setId(savedId);
                  setDirty(false);
                  publish();
                  emit saved(savedId);
              });
}

void WorkoutEditorViewModel::saveAsTemplate(const QString& name)
{
    if (!m_templateService || !m_editing)
        return;

    m_templateService->saveFromWorkout(m_workout, name)
        .onError(this, [this](const QString& error) { emit errorOccurred(error); })
        .then(this, [this](int templateId) { emit savedAsTemplate(templateId); });
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

Set WorkoutEditorViewModel::seedSetFor(const ExerciseDefinitionModel& definition) const
{
    const ExerciseDefinition& entity = definition.entity();
    const SetMetric metric = metricSuitsKind(entity.kind(), entity.defaultMetric())
        ? entity.defaultMetric()
        : defaultMetricFor(entity.kind());

    Set set;
    set.setMetric(metric);
    set.setLoadType(entity.defaultLoadType());

    switch (metric)
    {
        case SetMetric::Reps:
            set.setRepetitions(8);
            break;
        case SetMetric::Duration:
            set.setDurationSeconds(30);
            break;
        case SetMetric::Distance:
            set.setDistanceMeters(1000.0);
            break;
    }

    return set;
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
