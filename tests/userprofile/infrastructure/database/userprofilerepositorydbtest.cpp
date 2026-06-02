#include "modules/userprofile/infrastructure/database/userprofilerepositorydb.h"
#include "modules/userprofile/domain/entities/experiencelevel.h"
#include "modules/userprofile/domain/entities/primarygoal.h"
#include "modules/userprofile/domain/entities/sex.h"
#include "modules/userprofile/domain/entities/unitsystem.h"
#include "modules/userprofile/domain/entities/userprofile.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include <gtest/gtest.h>

class UserProfileRepositoryDbTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        m_database.setDatabaseName(":memory:");
        ASSERT_TRUE(m_database.open());

        m_dbStorage = std::make_unique<DbStorage>(m_database);
        m_repo = std::make_unique<UserProfileRepositoryDb>(*m_dbStorage);
        ASSERT_TRUE(m_repo->createTable());
    }

    void TearDown() override
    {
        m_repo.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    UserProfile makeProfile()
    {
        UserProfile p;
        p.setUserId(1);
        p.setLanguage("pl");
        p.setTimezone("Europe/Warsaw");
        p.setSex(Sex::Male);
        p.setSessionsPerWeek(4);
        p.setExperienceLevel(ExperienceLevel::Intermediate);
        p.setPrimaryGoal(PrimaryGoal::Strength);
        p.setBodyweightKg(82.0);
        p.setUnitSystem(UnitSystem::Metric);
        return p;
    }

    QString m_connectionName = "userprofile_repo_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<UserProfileRepositoryDb> m_repo;
};

TEST_F(UserProfileRepositoryDbTest, Find_EmptyDb_ReturnsNullopt)
{
    auto result = m_repo->find();

    EXPECT_FALSE(result.has_value());
}

TEST_F(UserProfileRepositoryDbTest, Save_And_Find_ReturnsProfile)
{
    UserProfile p = makeProfile();

    m_repo->save(p);
    auto found = m_repo->find();

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->userId(), 1);
    EXPECT_EQ(found->language(), "pl");
    EXPECT_EQ(found->timezone(), "Europe/Warsaw");
    EXPECT_EQ(found->sex(), Sex::Male);
    EXPECT_EQ(found->sessionsPerWeek(), 4);
    EXPECT_EQ(found->experienceLevel(), ExperienceLevel::Intermediate);
    EXPECT_EQ(found->primaryGoal(), PrimaryGoal::Strength);
    ASSERT_TRUE(found->bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(found->bodyweightKg().value(), 82.0);
    EXPECT_EQ(found->unitSystem(), UnitSystem::Metric);
}

TEST_F(UserProfileRepositoryDbTest, Save_Twice_OnlyOneRowExists)
{
    m_repo->save(makeProfile());
    m_repo->save(makeProfile());

    QSqlQuery q(m_database);
    q.exec("SELECT COUNT(*) FROM user_profile");
    q.next();

    EXPECT_EQ(q.value(0).toInt(), 1);
}

TEST_F(UserProfileRepositoryDbTest, Save_UpdatesExistingProfile)
{
    m_repo->save(makeProfile());

    UserProfile updated;
    updated.setUserId(1);
    updated.setLanguage("de");
    updated.setSex(Sex::Female);
    updated.setSessionsPerWeek(6);
    updated.setExperienceLevel(ExperienceLevel::Advanced);
    updated.setPrimaryGoal(PrimaryGoal::WeightLoss);
    updated.setBodyweightKg(65.0);
    updated.setUnitSystem(UnitSystem::Imperial);
    m_repo->save(updated);

    auto found = m_repo->find();
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->language(), "de");
    EXPECT_EQ(found->sex(), Sex::Female);
    EXPECT_EQ(found->sessionsPerWeek(), 6);
    EXPECT_EQ(found->experienceLevel(), ExperienceLevel::Advanced);
    EXPECT_EQ(found->primaryGoal(), PrimaryGoal::WeightLoss);
    ASSERT_TRUE(found->bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(found->bodyweightKg().value(), 65.0);
    EXPECT_EQ(found->unitSystem(), UnitSystem::Imperial);
}

TEST_F(UserProfileRepositoryDbTest, Save_NullBodyweight_StoresNull)
{
    UserProfile p;
    p.setUserId(1);

    m_repo->save(p);
    auto found = m_repo->find();

    ASSERT_TRUE(found.has_value());
    EXPECT_FALSE(found->bodyweightKg().has_value());
}

TEST_F(UserProfileRepositoryDbTest, Save_ClearBodyweight_UpdatesToNull)
{
    UserProfile p = makeProfile();
    m_repo->save(p);

    UserProfile updated;
    updated.setUserId(1);
    m_repo->save(updated);

    auto found = m_repo->find();
    ASSERT_TRUE(found.has_value());
    EXPECT_FALSE(found->bodyweightKg().has_value());
}

TEST_F(UserProfileRepositoryDbTest, AllEnumValues_RoundtripThroughDb)
{
    auto testEnum = [&](Sex sex, ExperienceLevel level, PrimaryGoal goal, UnitSystem unit)
    {
        UserProfile p;
        p.setUserId(1);
        p.setSex(sex);
        p.setExperienceLevel(level);
        p.setPrimaryGoal(goal);
        p.setUnitSystem(unit);
        m_repo->save(p);

        auto found = m_repo->find();
        ASSERT_TRUE(found.has_value());
        EXPECT_EQ(found->sex(), sex);
        EXPECT_EQ(found->experienceLevel(), level);
        EXPECT_EQ(found->primaryGoal(), goal);
        EXPECT_EQ(found->unitSystem(), unit);
    };

    testEnum(Sex::Male, ExperienceLevel::Beginner, PrimaryGoal::GeneralFitness, UnitSystem::Metric);
    testEnum(Sex::Female, ExperienceLevel::Intermediate, PrimaryGoal::WeightLoss,
             UnitSystem::Imperial);
    testEnum(Sex::Other, ExperienceLevel::Advanced, PrimaryGoal::MuscleGain, UnitSystem::Metric);
    testEnum(Sex::Male, ExperienceLevel::Beginner, PrimaryGoal::Strength, UnitSystem::Imperial);
    testEnum(Sex::Female, ExperienceLevel::Intermediate, PrimaryGoal::Endurance,
             UnitSystem::Metric);
}

TEST_F(UserProfileRepositoryDbTest, CreateTable_IsIdempotent)
{
    EXPECT_TRUE(m_repo->createTable());
    EXPECT_TRUE(m_repo->createTable());
}
