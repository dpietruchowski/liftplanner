#pragma once

#include <memory>
#include "modules/userprofile/domain/repositories/userprofilerepository.h"

class DbStorage;
class DbRepository;

class UserProfileRepositoryDb : public UserProfileRepository
{
public:
    explicit UserProfileRepositoryDb(DbStorage &storage);
    ~UserProfileRepositoryDb() override;

    bool createTable();

    std::optional<UserProfile> find() const override;
    void save(const UserProfile &profile) override;

private:
    std::unique_ptr<DbRepository> m_repo;
};
