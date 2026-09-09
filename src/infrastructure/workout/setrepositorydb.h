#pragma once

#include "domain/workout/set.h"
#include <QList>
#include <memory>
#include <vector>

class DbStorage;
class DbRepository;
class MigrationRunner;

class SetRepositoryDb
{
public:
    explicit SetRepositoryDb(DbStorage& storage);
    ~SetRepositoryDb();

    bool createTable();
    void registerMigrations(MigrationRunner& runner);

    std::vector<Set> findByExerciseId(int exerciseId) const;
    std::vector<Set> findByExerciseIds(const QList<int>& exerciseIds) const;
    int save(const Set& set);
    void removeByExerciseIdExcept(int exerciseId, const QList<int>& keptIds);

private:
    std::unique_ptr<DbRepository> m_repository;
};
