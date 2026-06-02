#pragma once

#include "modules/userprofile/domain/repositories/userprofilerepository.h"
#include <memory>

class DbStorage;
class DbRepository;
class MigrationRunner;

class UserProfileRepositoryDb : public UserProfileRepository
{
public:
    explicit UserProfileRepositoryDb(DbStorage& storage);
    ~UserProfileRepositoryDb() override;

    bool createTable();
    void registerMigrations(MigrationRunner& runner);

    std::optional<UserProfile> find() const override;
    void save(const UserProfile& profile) override;

private:
    std::unique_ptr<DbRepository> m_repo;
};
