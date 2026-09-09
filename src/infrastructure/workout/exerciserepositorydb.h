#pragma once

#include "domain/workout/exercise.h"
#include <QList>
#include <QStringList>
#include <dbtoolkit/query/where.h>
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
    void removeByWorkoutIdExcept(int workoutId, const QList<int>& keptIds);

private:
    static QStringList staticHoldNames();

    std::vector<Exercise> findBy(const Where& where) const;

    std::unique_ptr<DbRepository> m_repository;
};
