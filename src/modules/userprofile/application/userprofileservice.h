#pragma once

#include "modules/userprofile/domain/entities/userprofile.h"
#include <optional>

class UserProfileRepository;

class UserProfileService
{
public:
    explicit UserProfileService(UserProfileRepository& repository);

    std::optional<UserProfile> load() const;
    void save(const UserProfile& profile);
    bool exists() const;

private:
    UserProfileRepository& m_repository;
};
