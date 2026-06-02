#pragma once

#include "exerciserepositorydb.h"
#include "modules/workout/domain/repositories/workoutrepository.h"
#include "setrepositorydb.h"
#include <memory>
#include <vector>

#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/where.h>

class DbStorage;
class DbRepository;

class WorkoutRepositoryDb : public WorkoutRepository
{
public:
    explicit WorkoutRepositoryDb(DbStorage& storage);
    ~WorkoutRepositoryDb() override;

    bool createTables();

    std::vector<Workout> findAll(const WorkoutQuery& query) const override;
    std::optional<Workout> findOne(const WorkoutQuery& query) const override;
    int save(const Workout& workout) override;
    bool remove(const WorkoutQuery& query) override;
    int count(const WorkoutQuery& query) const override;
    bool exists(const WorkoutQuery& query) const override;

private:
    void loadChildren(Workout& workout) const;
    void saveChildren(int workoutId, const Workout& workout);

    Where buildWhereClause(const WorkoutQuery& query) const;
    Order buildOrderClause(const WorkoutQuery& query) const;

    std::unique_ptr<DbRepository> m_workoutRepo;
    ExerciseRepositoryDb m_exerciseRepo;
    SetRepositoryDb m_setRepo;
};
