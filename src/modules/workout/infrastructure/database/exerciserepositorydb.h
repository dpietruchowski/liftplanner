#pragma once

#include <memory>
#include <vector>
#include "modules/workout/domain/repositories/exerciserepository.h"

#include <dbtoolkit/query/where.h>

class DbStorage;
class DbRepository;

class ExerciseRepositoryDb : public ExerciseRepository
{
public:
    explicit ExerciseRepositoryDb(DbStorage &storage);
    ~ExerciseRepositoryDb() override;

    bool createTable();

    std::vector<Exercise> findAll(const ExerciseQuery &query) const override;
    std::optional<Exercise> findOne(const ExerciseQuery &query) const override;
    int save(const Exercise &exercise) override;
    bool remove(const ExerciseQuery &query) override;
    int count(const ExerciseQuery &query) const override;
    bool exists(const ExerciseQuery &query) const override;

private:
    std::unique_ptr<DbRepository> m_repository;

    Where buildWhereClause(const ExerciseQuery &query) const;
};
