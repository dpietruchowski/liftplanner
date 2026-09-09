#pragma once

#include <vector>

struct WorkoutTemplateRow;
class WorkoutTemplateQuery;

class WorkoutTemplateRowRepository
{
public:
    virtual ~WorkoutTemplateRowRepository() = default;

    virtual std::vector<WorkoutTemplateRow> findAll(const WorkoutTemplateQuery& query) const = 0;
};
