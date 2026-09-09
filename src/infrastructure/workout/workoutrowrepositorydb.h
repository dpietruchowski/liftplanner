#pragma once

#include "domain/workout/historysetrow.h"
#include "domain/workout/workoutrowrepository.h"

#include <vector>

class DbStorage;

class WorkoutRowRepositoryDb : public WorkoutRowRepository
{
public:
    explicit WorkoutRowRepositoryDb(DbStorage& storage);

    std::vector<HistorySetRow> findSetsOfRecentWorkouts(int workoutLimit) const override;

private:
    DbStorage& m_storage;
};
