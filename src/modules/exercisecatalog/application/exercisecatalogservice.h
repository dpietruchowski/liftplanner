#pragma once

#include "async/service.h"
#include "async/testing.h"
#include "modules/exercisecatalog/domain/entities/exercisedefinition.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionquery.h"

#include <QString>
#include <optional>
#include <vector>

class ExerciseDefinitionRepository;

class ExerciseCatalogService final : public Service
{
public:
    ExerciseCatalogService(ExerciseDefinitionRepository& repository, QObject* worker);

    Task<std::vector<ExerciseDefinition>> search(const ExerciseDefinitionQuery& query);
    Task<std::optional<ExerciseDefinition>> findById(int id);
    Task<std::optional<ExerciseDefinition>> findBySlug(const QString& slug);

    Task<int> saveCustom(const ExerciseDefinition& definition);
    Task<std::optional<int>> match(const QString& name);
    Task<int> matchOrImport(const QString& name, ExerciseKind kind);

    Task<bool> archive(int id);
    Task<bool> restore(int id);
    Task<bool> remove(int id);

private:
    LIBS_TEST_FRIEND(ExerciseCatalogServiceTest)

    Result<std::vector<ExerciseDefinition>> searchCore(ExerciseDefinitionQuery query);
    Result<std::optional<ExerciseDefinition>> findByIdCore(int id);
    Result<std::optional<ExerciseDefinition>> findBySlugCore(const QString& slug);
    Result<int> saveCustomCore(ExerciseDefinition definition);
    Result<std::optional<int>> matchCore(const QString& name);
    Result<int> matchOrImportCore(const QString& name, ExerciseKind kind);
    Result<bool> archiveCore(int id);
    Result<bool> restoreCore(int id);
    Result<bool> removeCore(int id);

    Result<bool> setArchived(int id, bool archived);
    std::vector<ExerciseDefinition> activeDefinitions() const;

    ExerciseDefinitionRepository& m_repository;
};
