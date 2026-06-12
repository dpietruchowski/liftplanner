#pragma once

#include "modules/workout/domain/entities/exercise.h"
#include <QList>
#include <memory>
#include <vector>

class DbStorage;
class DbRepository;
class MigrationRunner;

class ExerciseRepositoryDb
{
public:
    explicit ExerciseRepositoryDb(DbStorage& storage);
    ~ExerciseRepositoryDb();

    bool createTable();
    void registerMigrations(MigrationRunner& runner);

    std::vector<Exercise> findByWorkoutId(int workoutId) const;
    std::vector<Exercise> findByWorkoutIds(const QList<int>& workoutIds) const;
    int save(const Exercise& exercise);
    void removeByWorkoutId(int workoutId);

private:
    std::unique_ptr<DbRepository> m_repository;
};
