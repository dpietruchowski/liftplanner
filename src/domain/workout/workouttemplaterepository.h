#pragma once

#include <optional>
#include <vector>

class WorkoutTemplate;
class WorkoutTemplateQuery;

class WorkoutTemplateRepository
{
public:
    virtual ~WorkoutTemplateRepository() = default;

    virtual std::vector<WorkoutTemplate> findAll(const WorkoutTemplateQuery& query) const = 0;
    virtual std::optional<WorkoutTemplate> findOne(const WorkoutTemplateQuery& query) const = 0;
    virtual int save(const WorkoutTemplate& workoutTemplate) = 0;
    virtual bool remove(const WorkoutTemplateQuery& query) = 0;
    virtual int count(const WorkoutTemplateQuery& query) const = 0;
    virtual bool exists(const WorkoutTemplateQuery& query) const = 0;
};
