#pragma once

#include <optional>
#include <vector>

class Exercise;
class ExerciseQuery;

class ExerciseRepository
{
public:
    virtual ~ExerciseRepository() = default;

    virtual std::vector<Exercise> findAll(const ExerciseQuery &query) const = 0;
    virtual std::optional<Exercise> findOne(const ExerciseQuery &query) const = 0;
    virtual int save(const Exercise &exercise) = 0;
    virtual bool remove(const ExerciseQuery &query) = 0;
    virtual int count(const ExerciseQuery &query) const = 0;
    virtual bool exists(const ExerciseQuery &query) const = 0;
};
