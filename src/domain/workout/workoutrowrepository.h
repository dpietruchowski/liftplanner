#pragma once

#include <vector>

struct HistorySetRow;

class WorkoutRowRepository
{
public:
    virtual ~WorkoutRowRepository() = default;

    virtual std::vector<HistorySetRow> findSetsOfRecentWorkouts(int workoutLimit) const = 0;
};
