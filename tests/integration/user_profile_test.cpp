#include <QSignalSpy>
#include <gtest/gtest.h>

#include "testapplication.h"
#include "ui/viewmodels/userprofileviewmodel.h"

// =============================================================================
// User profile: loading defaults, dirty tracking, saving and reloading
// =============================================================================

class UserProfileTest : public ::testing::Test
{
protected:
    UserProfileViewModel& profile() { return app.userProfileViewModel(); }

    TestApplication app;
};

TEST_F(UserProfileTest, EmptyDatabase_LoadsDefaults)
{
    EXPECT_FALSE(profile().isDirty());
    EXPECT_FALSE(profile().timezone().isEmpty());
    EXPECT_FALSE(profile().language().isEmpty());
    EXPECT_EQ(profile().unitSystem(), "metric");
}

TEST_F(UserProfileTest, ChangingAField_MarksTheProfileDirty)
{
    QSignalSpy dirtySpy(&profile(), &UserProfileViewModel::dirtyChanged);

    profile().setSessionsPerWeek(5);

    EXPECT_TRUE(profile().isDirty());
    EXPECT_EQ(profile().sessionsPerWeek(), 5);
    EXPECT_EQ(dirtySpy.count(), 1);
}

TEST_F(UserProfileTest, SettingTheSameValue_LeavesItClean)
{
    profile().setSessionsPerWeek(profile().sessionsPerWeek());

    EXPECT_FALSE(profile().isDirty());
}

TEST_F(UserProfileTest, Save_AnnouncesOnlyWhenTheWriteCameBack)
{
    profile().setSessionsPerWeek(4);
    QSignalSpy savedSpy(&profile(), &UserProfileViewModel::saved);

    profile().save();
    EXPECT_EQ(savedSpy.count(), 0);

    app.drain();
    EXPECT_EQ(savedSpy.count(), 1);
}

TEST_F(UserProfileTest, Save_ClearsTheDirtyFlag)
{
    profile().setSessionsPerWeek(4);

    profile().save();
    app.drain();

    EXPECT_FALSE(profile().isDirty());
}

TEST_F(UserProfileTest, SavedProfile_ComesBackOnReload)
{
    profile().setSessionsPerWeek(6);
    profile().setPrimaryGoal(QStringLiteral("strength"));
    profile().setDateOfBirth(QStringLiteral("1990-05-14"));
    profile().setNotes(QStringLiteral("Bad left shoulder"));
    profile().save();
    app.drain();

    profile().load();
    app.drain();

    EXPECT_EQ(profile().sessionsPerWeek(), 6);
    EXPECT_EQ(profile().primaryGoal(), "strength");
    EXPECT_EQ(profile().dateOfBirth(), "1990-05-14");
    EXPECT_EQ(profile().notes(), "Bad left shoulder");
    EXPECT_FALSE(profile().isDirty());
}

TEST_F(UserProfileTest, ImperialUnits_ReportBodyweightInPounds)
{
    profile().setUnitSystem(QStringLiteral("metric"));
    profile().setBodyweight(80.0);

    profile().setUnitSystem(QStringLiteral("imperial"));

    EXPECT_EQ(profile().bodyweightUnit(), "lb");
    EXPECT_NEAR(profile().bodyweight(), 176.4, 0.05);
}

TEST_F(UserProfileTest, InvalidDateOfBirth_IsRejected)
{
    profile().setDateOfBirth(QStringLiteral("14-05-1990"));

    EXPECT_TRUE(profile().dateOfBirth().isEmpty());
    EXPECT_FALSE(profile().isDirty());
}

TEST_F(UserProfileTest, DateOfBirth_YieldsTheAge)
{
    app.setCurrentDate(QDate(2025, 6, 1));

    profile().setDateOfBirth(QStringLiteral("1990-05-14"));

    EXPECT_EQ(profile().age(), 35);
}
