#pragma once

#include "modules/workout/domain/repositories/exercisedefinitionlookup.h"

class ExerciseDefinitionRepository;

class CatalogDefinitionLookup final : public ExerciseDefinitionLookup
{
public:
    explicit CatalogDefinitionLookup(ExerciseDefinitionRepository& repository);

    std::optional<ExerciseDefinitionSnapshot> findDefinition(int definitionId) const override;

private:
    ExerciseDefinitionRepository& m_repository;
};
