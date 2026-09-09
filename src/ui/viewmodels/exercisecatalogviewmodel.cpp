#include "exercisecatalogviewmodel.h"

#include "modules/exercisecatalog/application/exercisecatalogservice.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionquery.h"

ExerciseCatalogViewModel::ExerciseCatalogViewModel(ExerciseCatalogService* service, QObject* parent)
    : QObject(parent)
    , m_service(service)
{
}

ExerciseCatalogViewModel::~ExerciseCatalogViewModel()
{
    qDeleteAll(m_exercises);
    m_exercises.clear();
}

QList<ExerciseDefinitionModel*> ExerciseCatalogViewModel::exercises() const { return m_exercises; }

int ExerciseCatalogViewModel::count() const { return static_cast<int>(m_exercises.size()); }

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
                  qDeleteAll(m_exercises);
                  m_exercises.clear();
                  for (const auto& definition : definitions)
                      m_exercises.append(new ExerciseDefinitionModel(definition, this));

                  setLoading(false);
                  emit exercisesChanged();
              })
        .onError(this,
                 [this](const QString& error)
                 {
                     setLoading(false);
                     emit errorOccurred(error);
                 });
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
    const QString trimmed = value.trimmed();
    if (target == trimmed)
        return;

    target = trimmed;
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
