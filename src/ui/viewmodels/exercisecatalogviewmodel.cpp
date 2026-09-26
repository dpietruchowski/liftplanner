#include "exercisecatalogviewmodel.h"

#include "application/exercisecatalog/exercisecatalogservice.h"
#include "domain/exercisecatalog/exercisedefinitionquery.h"
#include "ui/presentation/filterfield.h"
#include <algorithm>

namespace
{

constexpr int max_recent_shown = 8;

bool matches(const WorkoutService::RecentExercise& recent, const ExerciseDefinition& definition)
{
    if (recent.definitionId.has_value())
        return recent.definitionId.value() == definition.id();
    return recent.name.compare(definition.name(), Qt::CaseInsensitive) == 0;
}

}

ExerciseCatalogViewModel::ExerciseCatalogViewModel(ExerciseCatalogService* service,
                                                   WorkoutService* workoutService, QObject* parent)
    : QObject(parent)
    , m_service(service)
    , m_workoutService(workoutService)
{
}

ExerciseCatalogViewModel::~ExerciseCatalogViewModel()
{
    qDeleteAll(m_exercises);
    m_exercises.clear();
}

QList<ExerciseDefinitionModel*> ExerciseCatalogViewModel::exercises() const { return m_exercises; }

int ExerciseCatalogViewModel::count() const { return static_cast<int>(m_exercises.size()); }

int ExerciseCatalogViewModel::recentCount() const { return m_recentCount; }

bool ExerciseCatalogViewModel::isLoading() const { return m_loading; }

QString ExerciseCatalogViewModel::searchText() const { return m_searchText; }
QString ExerciseCatalogViewModel::muscle() const { return m_muscle; }
QString ExerciseCatalogViewModel::region() const { return m_region; }
QString ExerciseCatalogViewModel::equipment() const { return m_equipment; }
QString ExerciseCatalogViewModel::kind() const { return m_kind; }

bool ExerciseCatalogViewModel::isFiltered() const
{
    return !m_searchText.isEmpty() || !m_muscle.isEmpty() || !m_region.isEmpty()
        || !m_equipment.isEmpty() || !m_kind.isEmpty();
}

void ExerciseCatalogViewModel::setSearchText(const QString& value)
{
    applyFilterChange(m_searchText, value);
}

void ExerciseCatalogViewModel::setMuscle(const QString& value)
{
    applyFilterChange(m_muscle, value);
}

void ExerciseCatalogViewModel::setRegion(const QString& value)
{
    applyFilterChange(m_region, value);
}

void ExerciseCatalogViewModel::setEquipment(const QString& value)
{
    applyFilterChange(m_equipment, value);
}

void ExerciseCatalogViewModel::setKind(const QString& value) { applyFilterChange(m_kind, value); }

void ExerciseCatalogViewModel::load()
{
    if (!m_service)
        return;

    ExerciseDefinitionQuery query;
    if (!m_searchText.isEmpty())
        query.whereNameContains(m_searchText);
    if (!m_muscle.isEmpty())
        query.whereMuscle(muscleFromString(m_muscle));
    if (!m_region.isEmpty())
        query.whereRegion(bodyRegionFromString(m_region));
    if (!m_equipment.isEmpty())
        query.whereEquipment(equipmentFromString(m_equipment));
    if (!m_kind.isEmpty())
        query.whereKind(exerciseKindFromString(m_kind));

    setLoading(true);

    m_service->search(query)
        .then(this,
              [this](std::vector<ExerciseDefinition> definitions)
              {
                  showExercises(definitions);
                  setLoading(false);
              })
        .onError(this,
                 [this](const QString& error)
                 {
                     setLoading(false);
                     emit errorOccurred(error);
                 });
}

void ExerciseCatalogViewModel::refresh()
{
    if (!m_workoutService)
    {
        load();
        return;
    }

    m_workoutService->recentExercises()
        .then(this,
              [this](std::vector<WorkoutService::RecentExercise> recent)
              {
                  m_recent = std::move(recent);
                  load();
              })
        .onError(this,
                 [this](const QString& error)
                 {
                     emit errorOccurred(error);
                     load();
                 });
}

void ExerciseCatalogViewModel::showExercises(const std::vector<ExerciseDefinition>& definitions)
{
    QList<ExerciseDefinitionModel*> all;
    for (const auto& definition : definitions)
        all.append(new ExerciseDefinitionModel(definition, this));

    QList<ExerciseDefinitionModel*> ordered;
    for (const auto& entry : m_recent)
    {
        if (ordered.size() >= max_recent_shown)
            break;

        const auto found
            = std::find_if(all.begin(), all.end(), [&entry](const ExerciseDefinitionModel* model)
                           { return !model->isRecent() && matches(entry, model->entity()); });
        if (found == all.end())
            continue;

        (*found)->setLastPerformed(entry.performedAt);
        ordered.append(*found);
    }
    m_recentCount = static_cast<int>(ordered.size());

    for (auto* model : all)
    {
        if (!model->isRecent())
            ordered.append(model);
    }

    qDeleteAll(m_exercises);
    m_exercises = ordered;
    emit exercisesChanged();
}

void ExerciseCatalogViewModel::clearFilters()
{
    if (!isFiltered())
        return;

    m_searchText.clear();
    m_muscle.clear();
    m_region.clear();
    m_equipment.clear();
    m_kind.clear();

    emit filtersChanged();
    load();
}

ExerciseDefinitionModel* ExerciseCatalogViewModel::findById(int definitionId) const
{
    for (auto* model : m_exercises)
    {
        if (model->definitionId() == definitionId)
            return model;
    }
    return nullptr;
}

void ExerciseCatalogViewModel::applyFilterChange(QString& target, const QString& value)
{
    if (!FilterField::apply(target, value))
        return;

    emit filtersChanged();
    load();
}

void ExerciseCatalogViewModel::setLoading(bool value)
{
    if (m_loading == value)
        return;

    m_loading = value;
    emit loadingChanged();
}
