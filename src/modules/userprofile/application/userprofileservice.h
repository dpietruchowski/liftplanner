#pragma once

#include "modules/userprofile/domain/entities/userprofile.h"
#include "utils/service.h"
#include "utils/testing.h"
#include <optional>

class UserProfileRepository;

class UserProfileService final : public Service
{
public:
    UserProfileService(UserProfileRepository& repository, QObject* worker);

    Task<std::optional<UserProfile>> load();
    Task<void> save(const UserProfile& profile);
    Task<bool> exists();

private:
    LIBS_TEST_FRIEND(UserProfileServiceTest)

    Result<std::optional<UserProfile>> loadCore();
    Result<void> saveCore(const UserProfile& profile);
    Result<bool> existsCore();

    UserProfileRepository& m_repository;
};
