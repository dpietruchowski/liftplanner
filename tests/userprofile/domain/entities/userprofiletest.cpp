#include "modules/userprofile/domain/entities/userprofile.h"
#include "modules/userprofile/domain/entities/experiencelevel.h"
#include "modules/userprofile/domain/entities/primarygoal.h"
#include "modules/userprofile/domain/entities/sex.h"
#include "modules/userprofile/domain/entities/unitsystem.h"
#include <gtest/gtest.h>

class UserProfileTest : public ::testing::Test
{
};

// --- UserProfile entity ---

TEST_F(UserProfileTest, DefaultConstructor_SetsDefaults)
{
    UserProfile p;

    EXPECT_EQ(p.userId(), -1);
    EXPECT_EQ(p.language(), "en");
    EXPECT_EQ(p.timezone(), "UTC");
    EXPECT_EQ(p.sex(), Sex::Other);
    EXPECT_EQ(p.sessionsPerWeek(), 3);
    EXPECT_EQ(p.experienceLevel(), ExperienceLevel::Beginner);
    EXPECT_EQ(p.primaryGoal(), PrimaryGoal::GeneralFitness);
    EXPECT_FALSE(p.bodyweightKg().has_value());
    EXPECT_EQ(p.unitSystem(), UnitSystem::Metric);
}

TEST_F(UserProfileTest, CreateDefault_MatchesDefaultConstructor)
{
    UserProfile p = UserProfile::createDefault();

    EXPECT_EQ(p.userId(), -1);
    EXPECT_EQ(p.language(), "en");
    EXPECT_EQ(p.timezone(), "UTC");
    EXPECT_EQ(p.sex(), Sex::Other);
    EXPECT_EQ(p.sessionsPerWeek(), 3);
    EXPECT_EQ(p.experienceLevel(), ExperienceLevel::Beginner);
    EXPECT_EQ(p.primaryGoal(), PrimaryGoal::GeneralFitness);
    EXPECT_FALSE(p.bodyweightKg().has_value());
    EXPECT_EQ(p.unitSystem(), UnitSystem::Metric);
}

TEST_F(UserProfileTest, Setters_UpdateAllFields)
{
    UserProfile p;

    p.setUserId(1);
    p.setLanguage("pl");
    p.setTimezone("Europe/Warsaw");
    p.setSex(Sex::Male);
    p.setSessionsPerWeek(5);
    p.setExperienceLevel(ExperienceLevel::Advanced);
    p.setPrimaryGoal(PrimaryGoal::Strength);
    p.setBodyweightKg(85.5);
    p.setUnitSystem(UnitSystem::Imperial);

    EXPECT_EQ(p.userId(), 1);
    EXPECT_EQ(p.language(), "pl");
    EXPECT_EQ(p.timezone(), "Europe/Warsaw");
    EXPECT_EQ(p.sex(), Sex::Male);
    EXPECT_EQ(p.sessionsPerWeek(), 5);
    EXPECT_EQ(p.experienceLevel(), ExperienceLevel::Advanced);
    EXPECT_EQ(p.primaryGoal(), PrimaryGoal::Strength);
    ASSERT_TRUE(p.bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(p.bodyweightKg().value(), 85.5);
    EXPECT_EQ(p.unitSystem(), UnitSystem::Imperial);
}

TEST_F(UserProfileTest, BodyweightKg_SetAndClear)
{
    UserProfile p;
    EXPECT_FALSE(p.bodyweightKg().has_value());

    p.setBodyweightKg(70.0);
    ASSERT_TRUE(p.bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(p.bodyweightKg().value(), 70.0);

    p.setBodyweightKg(std::nullopt);
    EXPECT_FALSE(p.bodyweightKg().has_value());
}

TEST_F(UserProfileTest, CopySemantics)
{
    UserProfile original;
    original.setUserId(1);
    original.setLanguage("de");
    original.setSex(Sex::Female);
    original.setBodyweightKg(60.0);

    UserProfile copy = original;

    EXPECT_EQ(copy.userId(), 1);
    EXPECT_EQ(copy.language(), "de");
    EXPECT_EQ(copy.sex(), Sex::Female);
    ASSERT_TRUE(copy.bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(copy.bodyweightKg().value(), 60.0);
}

// --- Sex enum ---

TEST_F(UserProfileTest, Sex_ToStringFromString_Roundtrip)
{
    EXPECT_EQ(sexFromString(sexToString(Sex::Male)), Sex::Male);
    EXPECT_EQ(sexFromString(sexToString(Sex::Female)), Sex::Female);
    EXPECT_EQ(sexFromString(sexToString(Sex::Other)), Sex::Other);
}

TEST_F(UserProfileTest, Sex_ToString_CorrectValues)
{
    EXPECT_EQ(sexToString(Sex::Male), "male");
    EXPECT_EQ(sexToString(Sex::Female), "female");
    EXPECT_EQ(sexToString(Sex::Other), "other");
}

TEST_F(UserProfileTest, Sex_FromString_UnknownFallsToOther)
{
    EXPECT_EQ(sexFromString("unknown"), Sex::Other);
    EXPECT_EQ(sexFromString(""), Sex::Other);
}

// --- ExperienceLevel enum ---

TEST_F(UserProfileTest, ExperienceLevel_ToStringFromString_Roundtrip)
{
    EXPECT_EQ(experienceLevelFromString(experienceLevelToString(ExperienceLevel::Beginner)),
              ExperienceLevel::Beginner);
    EXPECT_EQ(experienceLevelFromString(experienceLevelToString(ExperienceLevel::Intermediate)),
              ExperienceLevel::Intermediate);
    EXPECT_EQ(experienceLevelFromString(experienceLevelToString(ExperienceLevel::Advanced)),
              ExperienceLevel::Advanced);
}

TEST_F(UserProfileTest, ExperienceLevel_ToString_CorrectValues)
{
    EXPECT_EQ(experienceLevelToString(ExperienceLevel::Beginner), "beginner");
    EXPECT_EQ(experienceLevelToString(ExperienceLevel::Intermediate), "intermediate");
    EXPECT_EQ(experienceLevelToString(ExperienceLevel::Advanced), "advanced");
}

TEST_F(UserProfileTest, ExperienceLevel_FromString_UnknownFallsToBeginner)
{
    EXPECT_EQ(experienceLevelFromString("unknown"), ExperienceLevel::Beginner);
    EXPECT_EQ(experienceLevelFromString(""), ExperienceLevel::Beginner);
}

// --- PrimaryGoal enum ---

TEST_F(UserProfileTest, PrimaryGoal_ToStringFromString_Roundtrip)
{
    EXPECT_EQ(primaryGoalFromString(primaryGoalToString(PrimaryGoal::GeneralFitness)),
              PrimaryGoal::GeneralFitness);
    EXPECT_EQ(primaryGoalFromString(primaryGoalToString(PrimaryGoal::WeightLoss)),
              PrimaryGoal::WeightLoss);
    EXPECT_EQ(primaryGoalFromString(primaryGoalToString(PrimaryGoal::MuscleGain)),
              PrimaryGoal::MuscleGain);
    EXPECT_EQ(primaryGoalFromString(primaryGoalToString(PrimaryGoal::Strength)),
              PrimaryGoal::Strength);
    EXPECT_EQ(primaryGoalFromString(primaryGoalToString(PrimaryGoal::Endurance)),
              PrimaryGoal::Endurance);
}

TEST_F(UserProfileTest, PrimaryGoal_ToString_CorrectValues)
{
    EXPECT_EQ(primaryGoalToString(PrimaryGoal::GeneralFitness), "general_fitness");
    EXPECT_EQ(primaryGoalToString(PrimaryGoal::WeightLoss), "weight_loss");
    EXPECT_EQ(primaryGoalToString(PrimaryGoal::MuscleGain), "muscle_gain");
    EXPECT_EQ(primaryGoalToString(PrimaryGoal::Strength), "strength");
    EXPECT_EQ(primaryGoalToString(PrimaryGoal::Endurance), "endurance");
}

TEST_F(UserProfileTest, PrimaryGoal_FromString_UnknownFallsToGeneralFitness)
{
    EXPECT_EQ(primaryGoalFromString("unknown"), PrimaryGoal::GeneralFitness);
    EXPECT_EQ(primaryGoalFromString(""), PrimaryGoal::GeneralFitness);
}

// --- UnitSystem enum ---

TEST_F(UserProfileTest, UnitSystem_ToStringFromString_Roundtrip)
{
    EXPECT_EQ(unitSystemFromString(unitSystemToString(UnitSystem::Metric)), UnitSystem::Metric);
    EXPECT_EQ(unitSystemFromString(unitSystemToString(UnitSystem::Imperial)), UnitSystem::Imperial);
}

TEST_F(UserProfileTest, UnitSystem_ToString_CorrectValues)
{
    EXPECT_EQ(unitSystemToString(UnitSystem::Metric), "metric");
    EXPECT_EQ(unitSystemToString(UnitSystem::Imperial), "imperial");
}

TEST_F(UserProfileTest, UnitSystem_FromString_UnknownFallsToMetric)
{
    EXPECT_EQ(unitSystemFromString("unknown"), UnitSystem::Metric);
    EXPECT_EQ(unitSystemFromString(""), UnitSystem::Metric);
}
