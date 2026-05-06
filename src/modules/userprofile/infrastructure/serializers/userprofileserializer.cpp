#include "userprofileserializer.h"
#include "modules/userprofile/domain/entities/userprofile.h"
#include "modules/userprofile/domain/entities/sex.h"
#include "modules/userprofile/domain/entities/experiencelevel.h"
#include "modules/userprofile/domain/entities/primarygoal.h"
#include "modules/userprofile/domain/entities/unitsystem.h"

UserProfile UserProfileSerializer::fromVariant(const QVariantMap &data)
{
    UserProfile profile;

    if (data.contains(user_id_key))
        profile.setUserId(data.value(user_id_key).toInt());
    if (data.contains(language_key))
        profile.setLanguage(data.value(language_key).toString());
    if (data.contains(timezone_key))
        profile.setTimezone(data.value(timezone_key).toString());
    if (data.contains(sex_key))
        profile.setSex(sexFromString(data.value(sex_key).toString()));
    if (data.contains(sessions_key))
        profile.setSessionsPerWeek(data.value(sessions_key).toInt());
    if (data.contains(experience_key))
        profile.setExperienceLevel(experienceLevelFromString(data.value(experience_key).toString()));
    if (data.contains(goal_key))
        profile.setPrimaryGoal(primaryGoalFromString(data.value(goal_key).toString()));
    if (data.contains(bodyweight_key) && !data.value(bodyweight_key).isNull())
        profile.setBodyweightKg(data.value(bodyweight_key).toDouble());
    if (data.contains(unit_system_key))
        profile.setUnitSystem(unitSystemFromString(data.value(unit_system_key).toString()));
    if (data.contains(notes_key))
        profile.setNotes(data.value(notes_key).toString());

    return profile;
}

QVariantMap UserProfileSerializer::toVariant(const UserProfile &profile)
{
    QVariantMap data;

    data.insert(user_id_key,    profile.userId());
    data.insert(language_key,   profile.language());
    data.insert(timezone_key,   profile.timezone());
    data.insert(sex_key,        sexToString(profile.sex()));
    data.insert(sessions_key,   profile.sessionsPerWeek());
    data.insert(experience_key, experienceLevelToString(profile.experienceLevel()));
    data.insert(goal_key,       primaryGoalToString(profile.primaryGoal()));
    data.insert(bodyweight_key, profile.bodyweightKg().has_value()
                                    ? QVariant(profile.bodyweightKg().value())
                                    : QVariant());
    data.insert(unit_system_key, unitSystemToString(profile.unitSystem()));
    data.insert(notes_key,       profile.notes());

    return data;
}
