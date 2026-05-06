#pragma once

#include <optional>

class UserProfile;

class UserProfileRepository
{
public:
    virtual ~UserProfileRepository() = default;

    virtual std::optional<UserProfile> find() const = 0;
    virtual void save(const UserProfile &profile) = 0;
};
