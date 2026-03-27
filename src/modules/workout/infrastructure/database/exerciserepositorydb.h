#pragma once

#include <memory>
#include <vector>
#include "modules/workout/domain/entities/exercise.h"

class DbStorage;
class DbRepository;

class ExerciseRepositoryDb
{
public:
    explicit ExerciseRepositoryDb(DbStorage &storage);
    ~ExerciseRepositoryDb();

    bool createTable();

    std::vector<Exercise> findByWorkoutId(int workoutId) const;
    int save(const Exercise &exercise);
    void removeByWorkoutId(int workoutId);

private:
    std::unique_ptr<DbRepository> m_repository;
};
