#pragma once

#include <optional>
#include "modules/userprofile/domain/entities/userprofile.h"

class UserProfileRepository;

class UserProfileService
{
public:
    explicit UserProfileService(UserProfileRepository &repository);

    std::optional<UserProfile> load() const;
    void save(const UserProfile &profile);
    bool exists() const;

private:
    UserProfileRepository &m_repository;
};
