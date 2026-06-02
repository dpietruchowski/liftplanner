#include "modules/userprofile/infrastructure/serializers/userprofileserializer.h"
#include "modules/userprofile/domain/entities/experiencelevel.h"
#include "modules/userprofile/domain/entities/primarygoal.h"
#include "modules/userprofile/domain/entities/sex.h"
#include "modules/userprofile/domain/entities/unitsystem.h"
#include "modules/userprofile/domain/entities/userprofile.h"
#include <gtest/gtest.h>

class UserProfileSerializerTest : public ::testing::Test
{
};

TEST_F(UserProfileSerializerTest, TableName_IsCorrect)
{
    EXPECT_STREQ(UserProfileSerializer::table, "user_profile");
}

TEST_F(UserProfileSerializerTest, ToVariant_ContainsAllKeys)
{
    UserProfile p;
    p.setUserId(1);

    QVariantMap data = UserProfileSerializer::toVariant(p);

    EXPECT_TRUE(data.contains(UserProfileSerializer::user_id_key));
    EXPECT_TRUE(data.contains(UserProfileSerializer::language_key));
    EXPECT_TRUE(data.contains(UserProfileSerializer::timezone_key));
    EXPECT_TRUE(data.contains(UserProfileSerializer::sex_key));
    EXPECT_TRUE(data.contains(UserProfileSerializer::sessions_key));
    EXPECT_TRUE(data.contains(UserProfileSerializer::experience_key));
    EXPECT_TRUE(data.contains(UserProfileSerializer::goal_key));
    EXPECT_TRUE(data.contains(UserProfileSerializer::bodyweight_key));
    EXPECT_TRUE(data.contains(UserProfileSerializer::unit_system_key));
}

TEST_F(UserProfileSerializerTest, ToVariant_SerializesDefaults)
{
    UserProfile p;
    p.setUserId(1);

    QVariantMap data = UserProfileSerializer::toVariant(p);

    EXPECT_EQ(data.value(UserProfileSerializer::user_id_key).toInt(), 1);
    EXPECT_EQ(data.value(UserProfileSerializer::language_key).toString(), "en");
    EXPECT_EQ(data.value(UserProfileSerializer::timezone_key).toString(), "UTC");
    EXPECT_EQ(data.value(UserProfileSerializer::sex_key).toString(), "other");
    EXPECT_EQ(data.value(UserProfileSerializer::sessions_key).toInt(), 3);
    EXPECT_EQ(data.value(UserProfileSerializer::experience_key).toString(), "beginner");
    EXPECT_EQ(data.value(UserProfileSerializer::goal_key).toString(), "general_fitness");
    EXPECT_TRUE(data.value(UserProfileSerializer::bodyweight_key).isNull());
    EXPECT_EQ(data.value(UserProfileSerializer::unit_system_key).toString(), "metric");
}

TEST_F(UserProfileSerializerTest, ToVariant_SerializesAllEnumValues)
{
    UserProfile p;
    p.setUserId(1);
    p.setSex(Sex::Female);
    p.setExperienceLevel(ExperienceLevel::Advanced);
    p.setPrimaryGoal(PrimaryGoal::MuscleGain);
    p.setUnitSystem(UnitSystem::Imperial);

    QVariantMap data = UserProfileSerializer::toVariant(p);

    EXPECT_EQ(data.value(UserProfileSerializer::sex_key).toString(), "female");
    EXPECT_EQ(data.value(UserProfileSerializer::experience_key).toString(), "advanced");
    EXPECT_EQ(data.value(UserProfileSerializer::goal_key).toString(), "muscle_gain");
    EXPECT_EQ(data.value(UserProfileSerializer::unit_system_key).toString(), "imperial");
}

TEST_F(UserProfileSerializerTest, ToVariant_BodyweightKg_PresentWhenSet)
{
    UserProfile p;
    p.setUserId(1);
    p.setBodyweightKg(90.5);

    QVariantMap data = UserProfileSerializer::toVariant(p);

    EXPECT_FALSE(data.value(UserProfileSerializer::bodyweight_key).isNull());
    EXPECT_DOUBLE_EQ(data.value(UserProfileSerializer::bodyweight_key).toDouble(), 90.5);
}

TEST_F(UserProfileSerializerTest, ToVariant_BodyweightKg_NullWhenNotSet)
{
    UserProfile p;
    p.setUserId(1);

    QVariantMap data = UserProfileSerializer::toVariant(p);

    EXPECT_TRUE(data.value(UserProfileSerializer::bodyweight_key).isNull());
}

TEST_F(UserProfileSerializerTest, FromVariant_DeserializesAllFields)
{
    QVariantMap data;
    data.insert(UserProfileSerializer::user_id_key, 1);
    data.insert(UserProfileSerializer::language_key, "pl");
    data.insert(UserProfileSerializer::timezone_key, "Europe/Warsaw");
    data.insert(UserProfileSerializer::sex_key, "male");
    data.insert(UserProfileSerializer::sessions_key, 4);
    data.insert(UserProfileSerializer::experience_key, "intermediate");
    data.insert(UserProfileSerializer::goal_key, "weight_loss");
    data.insert(UserProfileSerializer::bodyweight_key, 75.0);
    data.insert(UserProfileSerializer::unit_system_key, "imperial");

    UserProfile p = UserProfileSerializer::fromVariant(data);

    EXPECT_EQ(p.userId(), 1);
    EXPECT_EQ(p.language(), "pl");
    EXPECT_EQ(p.timezone(), "Europe/Warsaw");
    EXPECT_EQ(p.sex(), Sex::Male);
    EXPECT_EQ(p.sessionsPerWeek(), 4);
    EXPECT_EQ(p.experienceLevel(), ExperienceLevel::Intermediate);
    EXPECT_EQ(p.primaryGoal(), PrimaryGoal::WeightLoss);
    ASSERT_TRUE(p.bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(p.bodyweightKg().value(), 75.0);
    EXPECT_EQ(p.unitSystem(), UnitSystem::Imperial);
}

TEST_F(UserProfileSerializerTest, FromVariant_NullBodyweight_YieldsNullopt)
{
    QVariantMap data;
    data.insert(UserProfileSerializer::user_id_key, 1);
    data.insert(UserProfileSerializer::bodyweight_key, QVariant());

    UserProfile p = UserProfileSerializer::fromVariant(data);

    EXPECT_FALSE(p.bodyweightKg().has_value());
}

TEST_F(UserProfileSerializerTest, FromVariant_MissingBodyweight_YieldsNullopt)
{
    QVariantMap data;
    data.insert(UserProfileSerializer::user_id_key, 1);

    UserProfile p = UserProfileSerializer::fromVariant(data);

    EXPECT_FALSE(p.bodyweightKg().has_value());
}

TEST_F(UserProfileSerializerTest, FromVariant_EmptyMap_UsesEntityDefaults)
{
    UserProfile p = UserProfileSerializer::fromVariant({});

    EXPECT_EQ(p.userId(), -1);
    EXPECT_EQ(p.language(), "en");
    EXPECT_EQ(p.sex(), Sex::Other);
    EXPECT_EQ(p.experienceLevel(), ExperienceLevel::Beginner);
    EXPECT_EQ(p.primaryGoal(), PrimaryGoal::GeneralFitness);
    EXPECT_EQ(p.unitSystem(), UnitSystem::Metric);
}

TEST_F(UserProfileSerializerTest, Roundtrip_PreservesAllFields)
{
    UserProfile original;
    original.setUserId(1);
    original.setLanguage("fr");
    original.setTimezone("Europe/Paris");
    original.setSex(Sex::Female);
    original.setSessionsPerWeek(6);
    original.setExperienceLevel(ExperienceLevel::Intermediate);
    original.setPrimaryGoal(PrimaryGoal::Endurance);
    original.setBodyweightKg(58.3);
    original.setUnitSystem(UnitSystem::Metric);

    QVariantMap data = UserProfileSerializer::toVariant(original);
    UserProfile restored = UserProfileSerializer::fromVariant(data);

    EXPECT_EQ(restored.userId(), original.userId());
    EXPECT_EQ(restored.language(), original.language());
    EXPECT_EQ(restored.timezone(), original.timezone());
    EXPECT_EQ(restored.sex(), original.sex());
    EXPECT_EQ(restored.sessionsPerWeek(), original.sessionsPerWeek());
    EXPECT_EQ(restored.experienceLevel(), original.experienceLevel());
    EXPECT_EQ(restored.primaryGoal(), original.primaryGoal());
    ASSERT_TRUE(restored.bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(restored.bodyweightKg().value(), original.bodyweightKg().value());
    EXPECT_EQ(restored.unitSystem(), original.unitSystem());
}

TEST_F(UserProfileSerializerTest, Roundtrip_NullBodyweight)
{
    UserProfile original;
    original.setUserId(1);

    QVariantMap data = UserProfileSerializer::toVariant(original);
    UserProfile restored = UserProfileSerializer::fromVariant(data);

    EXPECT_FALSE(restored.bodyweightKg().has_value());
}
