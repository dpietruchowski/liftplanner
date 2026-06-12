#pragma once

#include "modules/workout/domain/entities/set.h"
#include <QList>
#include <memory>
#include <vector>

class DbStorage;
class DbRepository;

class SetRepositoryDb
{
public:
    explicit SetRepositoryDb(DbStorage& storage);
    ~SetRepositoryDb();

    bool createTable();

    std::vector<Set> findByExerciseId(int exerciseId) const;
    std::vector<Set> findByExerciseIds(const QList<int>& exerciseIds) const;
    int save(const Set& set);

private:
    std::unique_ptr<DbRepository> m_repository;
};
