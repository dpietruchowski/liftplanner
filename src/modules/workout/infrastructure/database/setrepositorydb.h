#pragma once

#include <memory>
#include <vector>
#include "modules/workout/domain/entities/set.h"

class DbStorage;
class DbRepository;

class SetRepositoryDb
{
public:
    explicit SetRepositoryDb(DbStorage &storage);
    ~SetRepositoryDb();

    bool createTable();

    std::vector<Set> findByExerciseId(int exerciseId) const;
    int save(const Set &set);
    void removeByExerciseId(int exerciseId);

private:
    std::unique_ptr<DbRepository> m_repository;
};
