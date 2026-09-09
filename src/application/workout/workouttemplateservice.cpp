#include "workouttemplateservice.h"

#include "domain/workout/exercisedefinitionlookup.h"
#include "domain/workout/setprescription.h"
#include "domain/workout/templateexercise.h"
#include "domain/workout/workouttemplatequery.h"
#include "domain/workout/workouttemplaterepository.h"
#include "domain/workout/workouttemplaterowrepository.h"

WorkoutTemplateService::WorkoutTemplateService(WorkoutTemplateRepository& repository,
                                               WorkoutTemplateRowRepository& rowRepository,
                                               const ExerciseDefinitionLookup& lookup,
                                               QObject* worker)
    : Service(worker)
    , m_repository(repository)
    , m_rowRepository(rowRepository)
    , m_lookup(lookup)
{
}

Task<std::vector<WorkoutTemplate>> WorkoutTemplateService::loadTemplates()
{
    return invoke([this] { return loadTemplatesCore(); });
}

Task<std::vector<WorkoutTemplate>> WorkoutTemplateService::searchTemplates(const QString& text)
{
    return invoke([this, text] { return searchTemplatesCore(text); });
}

Task<std::vector<WorkoutTemplateRow>> WorkoutTemplateService::searchRows(const QString& text)
{
    return invoke([this, text] { return searchRowsCore(text); });
}

Task<std::optional<WorkoutTemplate>> WorkoutTemplateService::findById(int id)
{
    return invoke([this, id] { return findByIdCore(id); });
}

Task<int> WorkoutTemplateService::save(const WorkoutTemplate& workoutTemplate)
{
    return invoke([this, workoutTemplate] { return saveCore(workoutTemplate); });
}

Task<int> WorkoutTemplateService::saveFromWorkout(const Workout& workout, const QString& name)
{
    return invoke([this, workout, name] { return saveFromWorkoutCore(workout, name); });
}

Task<int> WorkoutTemplateService::duplicate(int id, const QString& name)
{
    return invoke([this, id, name] { return duplicateCore(id, name); });
}

Task<bool> WorkoutTemplateService::remove(int id)
{
    return invoke([this, id] { return removeCore(id); });
}

Task<Workout> WorkoutTemplateService::instantiate(int id, const QDateTime& plannedTime)
{
    return invoke([this, id, plannedTime] { return instantiateCore(id, plannedTime); });
}

Result<std::vector<WorkoutTemplate>> WorkoutTemplateService::loadTemplatesCore()
{
    WorkoutTemplateQuery query;
    query.orderByName(SortDirection::Ascending);

    return Result<std::vector<WorkoutTemplate>>::success(m_repository.findAll(query));
}

Result<std::vector<WorkoutTemplate>>
WorkoutTemplateService::searchTemplatesCore(const QString& text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return loadTemplatesCore();

    WorkoutTemplateQuery query;
    query.whereNameContains(trimmed).orderByName(SortDirection::Ascending);

    return Result<std::vector<WorkoutTemplate>>::success(m_repository.findAll(query));
}

Result<std::vector<WorkoutTemplateRow>> WorkoutTemplateService::searchRowsCore(const QString& text)
{
    WorkoutTemplateQuery query;
    query.orderByName(SortDirection::Ascending);

    const QString trimmed = text.trimmed();
    if (!trimmed.isEmpty())
        query.whereNameContains(trimmed);

    return Result<std::vector<WorkoutTemplateRow>>::success(m_rowRepository.findAll(query));
}

Result<std::optional<WorkoutTemplate>> WorkoutTemplateService::findByIdCore(int id)
{
    return Result<std::optional<WorkoutTemplate>>::success(
        m_repository.findOne(WorkoutTemplateQuery().whereId(id)));
}

Result<int> WorkoutTemplateService::saveCore(WorkoutTemplate workoutTemplate)
{
    workoutTemplate.setName(workoutTemplate.name().trimmed());
    workoutTemplate.normalizePositions();

    const QStringList errors = workoutTemplate.validationErrors();
    if (!errors.isEmpty())
        return Result<int>::failure(errors.join(QStringLiteral("; ")));

    const auto clash
        = m_repository.findOne(WorkoutTemplateQuery().whereName(workoutTemplate.name()));
    if (clash.has_value() && clash->id() != workoutTemplate.id())
    {
        return Result<int>::failure(
            QStringLiteral("a template named %1 already exists").arg(workoutTemplate.name()));
    }

    return Result<int>::success(m_repository.save(workoutTemplate));
}

Result<int> WorkoutTemplateService::saveFromWorkoutCore(const Workout& workout, const QString& name)
{
    WorkoutTemplate workoutTemplate(name.trimmed().isEmpty() ? workout.name() : name);

    for (const auto& exercise : workout.exercises())
    {
        if (!exercise.hasDefinition())
        {
            return Result<int>::failure(
                QStringLiteral("%1 is not linked to the exercise catalog and cannot be templated")
                    .arg(exercise.name()));
        }

        TemplateExercise templateExercise(exercise.definitionId().value());
        templateExercise.setRestSecondsOverride(exercise.restSeconds());
        templateExercise.setNotes(exercise.notes());

        for (const auto& set : exercise.sets())
            templateExercise.addSet(SetPrescription::fromSet(set));

        workoutTemplate.addExercise(templateExercise);
    }

    return saveCore(workoutTemplate);
}

Result<int> WorkoutTemplateService::duplicateCore(int id, const QString& name)
{
    const auto source = m_repository.findOne(WorkoutTemplateQuery().whereId(id));
    if (!source.has_value())
        return Result<int>::failure(QStringLiteral("template %1 does not exist").arg(id));

    WorkoutTemplate copy = source.value();
    copy.setId(-1);
    copy.setName(name.trimmed().isEmpty() ? QStringLiteral("%1 (copy)").arg(source->name()) : name);

    return saveCore(copy);
}

Result<bool> WorkoutTemplateService::removeCore(int id)
{
    if (!m_repository.exists(WorkoutTemplateQuery().whereId(id)))
        return Result<bool>::success(false);

    return Result<bool>::success(m_repository.remove(WorkoutTemplateQuery().whereId(id)));
}

Result<Workout> WorkoutTemplateService::instantiateCore(int id, const QDateTime& plannedTime)
{
    const auto source = m_repository.findOne(WorkoutTemplateQuery().whereId(id));
    if (!source.has_value())
        return Result<Workout>::failure(QStringLiteral("template %1 does not exist").arg(id));

    Workout workout(source->name(), QDateTime::currentDateTime());
    workout.setPlannedTime(plannedTime);
    workout.setStatus(WorkoutStatus::Planned);

    for (const auto& templateExercise : source->exercises())
    {
        const auto definition = m_lookup.findDefinition(templateExercise.definitionId());
        if (!definition.has_value())
        {
            return Result<Workout>::failure(
                QStringLiteral("exercise definition %1 is missing from the catalog")
                    .arg(templateExercise.definitionId()));
        }

        workout.addExercise(templateExercise.toExercise(definition->name, definition->kind,
                                                        definition->restSeconds));
    }

    return Result<Workout>::success(workout);
}
