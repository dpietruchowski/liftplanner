#pragma once

#include <optional>
#include <vector>

class Workout;
class WorkoutQuery;

class WorkoutRepository
{
public:
    virtual ~WorkoutRepository() = default;

    virtual std::vector<Workout> findAll(const WorkoutQuery &query) const = 0;
    virtual std::optional<Workout> findOne(const WorkoutQuery &query) const = 0;
    virtual int save(const Workout &workout) = 0;
    virtual bool remove(const WorkoutQuery &query) = 0;
    virtual int count(const WorkoutQuery &query) const = 0;
    virtual bool exists(const WorkoutQuery &query) const = 0;
};
