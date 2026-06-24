#include "userprofileservice.h"
#include "modules/userprofile/domain/repositories/userprofilerepository.h"

UserProfileService::UserProfileService(UserProfileRepository& repository, QObject* worker)
    : Service(worker)
    , m_repository(repository)
{
}

Task<std::optional<UserProfile>> UserProfileService::load()
{
    return invoke([this] { return loadCore(); });
}

Task<void> UserProfileService::save(const UserProfile& profile)
{
    return invoke([this, profile] { return saveCore(profile); });
}

Task<bool> UserProfileService::exists()
{
    return invoke([this] { return existsCore(); });
}

Result<std::optional<UserProfile>> UserProfileService::loadCore()
{
    return Result<std::optional<UserProfile>>::success(m_repository.find());
}

Result<void> UserProfileService::saveCore(const UserProfile& profile)
{
    m_repository.save(profile);
    return Result<void>::success();
}

Result<bool> UserProfileService::existsCore()
{
    return Result<bool>::success(m_repository.find().has_value());
}
