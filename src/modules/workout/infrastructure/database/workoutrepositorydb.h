#pragma once

#include <memory>
#include <vector>
#include "modules/workout/domain/repositories/workoutrepository.h"

#include <dbtoolkit/query/where.h>
#include <dbtoolkit/query/order.h>

class DbStorage;
class DbRepository;

class WorkoutRepositoryDb : public WorkoutRepository
{
public:
    explicit WorkoutRepositoryDb(DbStorage &storage);
    ~WorkoutRepositoryDb() override;

    bool createTable();

    std::vector<Workout> findAll(const WorkoutQuery &query) const override;
    std::optional<Workout> findOne(const WorkoutQuery &query) const override;
    int save(const Workout &workout) override;
    bool remove(const WorkoutQuery &query) override;
    int count(const WorkoutQuery &query) const override;
    bool exists(const WorkoutQuery &query) const override;

private:
    std::unique_ptr<DbRepository> m_repository;

    Where buildWhereClause(const WorkoutQuery &query) const;
    Order buildOrderClause(const WorkoutQuery &query) const;
};
