#include "catalogdefinitionlookup.h"

#include "domain/exercisecatalog/exercisedefinition.h"
#include "domain/exercisecatalog/exercisedefinitionquery.h"
#include "domain/exercisecatalog/exercisedefinitionrepository.h"

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
