#pragma once

#include <QVariantMap>

class UserProfile;

class UserProfileSerializer
{
public:
    static constexpr const char* table = "user_profile";
    static constexpr const char* user_id_key = "user_id";
    static constexpr const char* language_key = "language";
    static constexpr const char* timezone_key = "timezone";
    static constexpr const char* sex_key = "sex";
    static constexpr const char* sessions_key = "sessions_per_week";
    static constexpr const char* experience_key = "experience_level";
    static constexpr const char* goal_key = "primary_goal";
    static constexpr const char* date_of_birth_key = "date_of_birth";
    static constexpr const char* bodyweight_key = "bodyweight_kg";
    static constexpr const char* unit_system_key = "unit_system";
    static constexpr const char* notes_key = "notes";

    static UserProfile fromVariant(const QVariantMap& data);
    static QVariantMap toVariant(const UserProfile& profile);
};
