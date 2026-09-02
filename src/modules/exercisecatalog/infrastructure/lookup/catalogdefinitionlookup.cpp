#include "catalogdefinitionlookup.h"

#include "modules/exercisecatalog/domain/entities/exercisedefinition.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionquery.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionrepository.h"

CatalogDefinitionLookup::CatalogDefinitionLookup(ExerciseDefinitionRepository& repository)
    : m_repository(repository)
{
}

std::optional<ExerciseDefinitionSnapshot>
CatalogDefinitionLookup::findDefinition(int definitionId) const
{
    const auto definition = m_repository.findOne(ExerciseDefinitionQuery().whereId(definitionId));
    if (!definition.has_value())
        return std::nullopt;

    return ExerciseDefinitionSnapshot { definition->id(), definition->name(), definition->kind(),
                                        definition->defaultRestSeconds() };
}
