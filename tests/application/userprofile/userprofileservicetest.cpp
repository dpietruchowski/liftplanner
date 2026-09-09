#include "application/userprofile/userprofileservice.h"
#include "domain/userprofile/experiencelevel.h"
#include "domain/userprofile/primarygoal.h"
#include "domain/userprofile/sex.h"
#include "domain/userprofile/userprofile.h"
#include "domain/userprofile/userprofilerepository.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockUserProfileRepository : public UserProfileRepository
{
public:
    MOCK_METHOD(std::optional<UserProfile>, find, (), (const, override));
    MOCK_METHOD(void, save, (const UserProfile& profile), (override));
};

class UserProfileServiceTest : public ::testing::Test
{
protected:
    void SetUp() override { m_service = std::make_unique<UserProfileService>(m_repo, nullptr); }

    std::optional<UserProfile> load() { return m_service->loadCore().value(); }
    void save(const UserProfile& profile) { m_service->saveCore(profile); }
    bool exists() { return m_service->existsCore().value(); }

    UserProfile makeProfile()
    {
        UserProfile p;
        p.setUserId(1);
        p.setLanguage("pl");
        p.setSex(Sex::Male);
        p.setSessionsPerWeek(3);
        p.setExperienceLevel(ExperienceLevel::Intermediate);
        p.setPrimaryGoal(PrimaryGoal::Strength);
        p.setBodyweightKg(80.0);
        return p;
    }

    MockUserProfileRepository m_repo;
    std::unique_ptr<UserProfileService> m_service;
};

// --- load ---

TEST_F(UserProfileServiceTest, Load_DelegatesToRepository)
{
    EXPECT_CALL(m_repo, find()).WillOnce(::testing::Return(std::nullopt));

    load();
}

TEST_F(UserProfileServiceTest, Load_ReturnsProfileFromRepository)
{
    UserProfile p = makeProfile();

    EXPECT_CALL(m_repo, find()).WillOnce(::testing::Return(std::optional<UserProfile> { p }));

    auto result = load();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->userId(), 1);
    EXPECT_EQ(result->language(), "pl");
    EXPECT_EQ(result->sex(), Sex::Male);
    EXPECT_EQ(result->sessionsPerWeek(), 3);
    EXPECT_EQ(result->experienceLevel(), ExperienceLevel::Intermediate);
    EXPECT_EQ(result->primaryGoal(), PrimaryGoal::Strength);
    ASSERT_TRUE(result->bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(result->bodyweightKg().value(), 80.0);
}

TEST_F(UserProfileServiceTest, Load_ReturnsNulloptWhenNoProfile)
{
    EXPECT_CALL(m_repo, find()).WillOnce(::testing::Return(std::nullopt));

    auto result = load();

    EXPECT_FALSE(result.has_value());
}

// --- save ---

TEST_F(UserProfileServiceTest, Save_DelegatesToRepository)
{
    UserProfile p = makeProfile();

    EXPECT_CALL(m_repo, save(::testing::_))
        .WillOnce(
            [](const UserProfile& saved)
            {
                EXPECT_EQ(saved.userId(), 1);
                EXPECT_EQ(saved.language(), "pl");
            });

    save(p);
}

TEST_F(UserProfileServiceTest, Save_PassesProfileUnmodified)
{
    UserProfile p = makeProfile();
    p.setBodyweightKg(92.5);
    p.setUnitSystem(UnitSystem::Imperial);

    std::optional<UserProfile> captured;
    EXPECT_CALL(m_repo, save(::testing::_))
        .WillOnce([&captured](const UserProfile& saved) { captured = saved; });

    save(p);

    ASSERT_TRUE(captured.has_value());
    ASSERT_TRUE(captured->bodyweightKg().has_value());
    EXPECT_DOUBLE_EQ(captured->bodyweightKg().value(), 92.5);
    EXPECT_EQ(captured->unitSystem(), UnitSystem::Imperial);
}

// --- exists ---

TEST_F(UserProfileServiceTest, Exists_ReturnsTrueWhenProfileFound)
{
    EXPECT_CALL(m_repo, find())
        .WillOnce(::testing::Return(std::optional<UserProfile> { makeProfile() }));

    EXPECT_TRUE(exists());
}

TEST_F(UserProfileServiceTest, Exists_ReturnsFalseWhenNoProfile)
{
    EXPECT_CALL(m_repo, find()).WillOnce(::testing::Return(std::nullopt));

    EXPECT_FALSE(exists());
}
