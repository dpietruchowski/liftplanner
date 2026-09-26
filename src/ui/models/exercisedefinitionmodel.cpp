#include "exercisedefinitionmodel.h"

ExerciseDefinitionModel::ExerciseDefinitionModel(QObject* parent)
    : QObject(parent)
{
}

ExerciseDefinitionModel::ExerciseDefinitionModel(const ExerciseDefinition& definition,
                                                 QObject* parent)
    : QObject(parent)
    , m_definition(definition)
{
}

int ExerciseDefinitionModel::definitionId() const { return m_definition.id(); }
QString ExerciseDefinitionModel::slug() const { return m_definition.slug(); }
QString ExerciseDefinitionModel::name() const { return m_definition.name(); }
QString ExerciseDefinitionModel::kind() const { return exerciseKindToString(m_definition.kind()); }

QString ExerciseDefinitionModel::equipment() const
{
    return equipmentToString(m_definition.equipment());
}

QString ExerciseDefinitionModel::defaultMetric() const
{
    return setMetricToString(m_definition.defaultMetric());
}

QString ExerciseDefinitionModel::defaultLoadType() const
{
    return loadTypeToString(m_definition.defaultLoadType());
}

int ExerciseDefinitionModel::defaultRestSeconds() const
{
    return m_definition.defaultRestSeconds();
}

QStringList ExerciseDefinitionModel::primaryMuscles() const
{
    return musclesWithRole(MuscleRole::Primary);
}

QStringList ExerciseDefinitionModel::secondaryMuscles() const
{
    return musclesWithRole(MuscleRole::Secondary);
}

QStringList ExerciseDefinitionModel::regions() const
{
    QStringList names;
    for (const auto region : m_definition.regions())
        names.append(bodyRegionToString(region));
    return names;
}

QString ExerciseDefinitionModel::videoUrl() const { return m_definition.videoUrl(); }
QString ExerciseDefinitionModel::instructions() const { return m_definition.instructions(); }

bool ExerciseDefinitionModel::isCustom() const
{
    return m_definition.origin() != CatalogOrigin::BuiltIn;
}

bool ExerciseDefinitionModel::isArchived() const { return m_definition.isArchived(); }

QDateTime ExerciseDefinitionModel::lastPerformed() const { return m_lastPerformed; }

bool ExerciseDefinitionModel::isRecent() const { return m_lastPerformed.isValid(); }

void ExerciseDefinitionModel::setLastPerformed(const QDateTime& performedAt)
{
    if (m_lastPerformed == performedAt)
        return;

    m_lastPerformed = performedAt;
    emit dataChanged();
}

const ExerciseDefinition& ExerciseDefinitionModel::entity() const { return m_definition; }

QStringList ExerciseDefinitionModel::musclesWithRole(MuscleRole role) const
{
    QStringList names;
    for (const auto muscle : m_definition.musclesWithRole(role))
        names.append(muscleToString(muscle));
    return names;
}
