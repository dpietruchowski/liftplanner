#pragma once

#include "domain/exercisecatalog/exercisedefinition.h"
#include "domain/exercisecatalog/exercisedefinitionrepository.h"

#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/where.h>

#include <QList>
#include <memory>
#include <vector>

class DbStorage;
class DbRepository;
class MigrationRunner;

class ExerciseDefinitionRepositoryDb : public ExerciseDefinitionRepository
{
public:
    explicit ExerciseDefinitionRepositoryDb(DbStorage& storage);
    ~ExerciseDefinitionRepositoryDb() override;

    bool createTables();
    void registerMigrations(MigrationRunner& runner);

    std::vector<ExerciseDefinition> findAll(const ExerciseDefinitionQuery& query) const override;
    std::optional<ExerciseDefinition> findOne(const ExerciseDefinitionQuery& query) const override;
    int save(const ExerciseDefinition& definition) override;
    bool remove(const ExerciseDefinitionQuery& query) override;
    int count(const ExerciseDefinitionQuery& query) const override;
    bool exists(const ExerciseDefinitionQuery& query) const override;

private:
    void loadMuscles(ExerciseDefinition& definition) const;
    void loadMuscles(std::vector<ExerciseDefinition>& definitions) const;
    void saveMuscles(int definitionId, const ExerciseDefinition& definition);

    int findIdBySlug(const QString& slug) const;

    Where buildWhereClause(const ExerciseDefinitionQuery& query) const;
    Order buildOrderClause(const ExerciseDefinitionQuery& query) const;

    std::unique_ptr<DbRepository> m_definitionRepo;
    std::unique_ptr<DbRepository> m_muscleRepo;
};
