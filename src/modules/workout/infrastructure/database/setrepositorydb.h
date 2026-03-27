#pragma once

#include <memory>
#include <vector>
#include "modules/workout/domain/repositories/setrepository.h"

#include <dbtoolkit/query/where.h>

class DbStorage;
class DbRepository;

class SetRepositoryDb : public SetRepository
{
public:
    explicit SetRepositoryDb(DbStorage &storage);
    ~SetRepositoryDb() override;

    bool createTable();

    std::vector<Set> findAll(const SetQuery &query) const override;
    std::optional<Set> findOne(const SetQuery &query) const override;
    int save(const Set &set) override;
    bool remove(const SetQuery &query) override;
    int count(const SetQuery &query) const override;
    bool exists(const SetQuery &query) const override;

private:
    std::unique_ptr<DbRepository> m_repository;

    Where buildWhereClause(const SetQuery &query) const;
};
