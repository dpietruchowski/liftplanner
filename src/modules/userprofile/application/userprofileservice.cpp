#include "userprofileservice.h"
#include "modules/userprofile/domain/repositories/userprofilerepository.h"

UserProfileService::UserProfileService(UserProfileRepository &repository)
    : m_repository(repository)
{
}

std::optional<UserProfile> UserProfileService::load() const
{
    return m_repository.find();
}

void UserProfileService::save(const UserProfile &profile)
{
    m_repository.save(profile);
}

bool UserProfileService::exists() const
{
    return m_repository.find().has_value();
}
