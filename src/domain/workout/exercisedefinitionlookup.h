#pragma once

#include "domain/workout/exercisekind.h"
#include <QString>
#include <optional>

struct ExerciseDefinitionSnapshot final
{
    int definitionId { -1 };
    QString name;
    ExerciseKind kind { ExerciseKind::Strength };
    int restSeconds { 120 };
};

class ExerciseDefinitionLookup
{
public:
    virtual ~ExerciseDefinitionLookup() = default;

    virtual std::optional<ExerciseDefinitionSnapshot> findDefinition(int definitionId) const = 0;
};
