#pragma once

#include "domain/workout/exercisedefinitionlookup.h"

class ExerciseDefinitionRepository;

class CatalogDefinitionLookup final : public ExerciseDefinitionLookup
{
public:
    explicit CatalogDefinitionLookup(ExerciseDefinitionRepository& repository);

    std::optional<ExerciseDefinitionSnapshot> findDefinition(int definitionId) const override;

private:
    ExerciseDefinitionRepository& m_repository;
};
