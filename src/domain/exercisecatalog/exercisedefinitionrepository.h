#pragma once

#include <optional>
#include <vector>

class ExerciseDefinition;
class ExerciseDefinitionQuery;

class ExerciseDefinitionRepository
{
public:
    virtual ~ExerciseDefinitionRepository() = default;

    virtual std::vector<ExerciseDefinition> findAll(const ExerciseDefinitionQuery& query) const = 0;
    virtual std::optional<ExerciseDefinition> findOne(const ExerciseDefinitionQuery& query) const
        = 0;
    virtual int save(const ExerciseDefinition& definition) = 0;
    virtual bool remove(const ExerciseDefinitionQuery& query) = 0;
    virtual int count(const ExerciseDefinitionQuery& query) const = 0;
    virtual bool exists(const ExerciseDefinitionQuery& query) const = 0;
};
