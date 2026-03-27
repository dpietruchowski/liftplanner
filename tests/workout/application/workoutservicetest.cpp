#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "modules/workout/application/workoutservice.h"
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/repositories/workoutrepository.h"
#include "modules/workout/domain/repositories/workoutquery.h"

class MockWorkoutRepository : public WorkoutRepository
{
public:
    MOCK_METHOD(std::vector<Workout>, findAll, (const WorkoutQuery &query), (const, override));
    MOCK_METHOD(std::optional<Workout>, findOne, (const WorkoutQuery &query), (const, override));
    MOCK_METHOD(int, save, (const Workout &workout), (override));
    MOCK_METHOD(bool, remove, (const WorkoutQuery &query), (override));
    MOCK_METHOD(int, count, (const WorkoutQuery &query), (const, override));
    MOCK_METHOD(bool, exists, (const WorkoutQuery &query), (const, override));
};

class WorkoutServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_service = std::make_unique<WorkoutService>(m_repo);
    }

    Workout makePlannedWorkout(const QString &name, int daysFromNow = 1)
    {
        Workout w(name, QDateTime::currentDateTime());
        w.setPlannedTime(QDateTime::currentDateTime().addDays(daysFromNow));
        return w;
    }

    Workout makeStartedWorkout(const QString &name)
    {
        Workout w(name, QDateTime::currentDateTime());
        w.setStartedTime(QDateTime::currentDateTime());
        return w;
    }

    Workout makeCompletedWorkout(const QString &name)
    {
        Workout w(name, QDateTime::currentDateTime());
        w.setStartedTime(QDateTime::currentDateTime().addSecs(-3600));
        w.setEndedTime(QDateTime::currentDateTime());
        return w;
    }

    MockWorkoutRepository m_repo;
    std::unique_ptr<WorkoutService> m_service;
};

// --- loadPlannedWorkouts ---

TEST_F(WorkoutServiceTest, LoadPlannedWorkouts_QueriesStartedTimeIsNull)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce([](const WorkoutQuery &query)
                  {
            EXPECT_TRUE(query.startedTimeIsNull().has_value());
            EXPECT_TRUE(query.startedTimeIsNull().value());
            return std::vector<Workout>{}; });

    m_service->loadPlannedWorkouts();
}

TEST_F(WorkoutServiceTest, LoadPlannedWorkouts_OrdersByPlannedTimeAscending)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce([](const WorkoutQuery &query)
                  {
            EXPECT_TRUE(query.orderByPlannedTimeDirection().has_value());
            EXPECT_EQ(query.orderByPlannedTimeDirection().value(), SortDirection::Ascending);
            return std::vector<Workout>{}; });

    m_service->loadPlannedWorkouts();
}

TEST_F(WorkoutServiceTest, LoadPlannedWorkouts_ReturnsWorkoutsFromRepo)
{
    std::vector<Workout> planned = {makePlannedWorkout("Day A"), makePlannedWorkout("Day B")};

    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce(::testing::Return(planned));

    auto result = m_service->loadPlannedWorkouts();
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].name(), "Day A");
    EXPECT_EQ(result[1].name(), "Day B");
}

// --- importPlannedWorkouts ---

TEST_F(WorkoutServiceTest, ImportPlannedWorkouts_RemovesExistingFirst)
{
    ::testing::InSequence seq;

    EXPECT_CALL(m_repo, remove(::testing::_))
        .WillOnce([](const WorkoutQuery &query)
                  {
            EXPECT_TRUE(query.startedTimeIsNull().has_value());
            EXPECT_TRUE(query.startedTimeIsNull().value());
            return true; });

    EXPECT_CALL(m_repo, save(::testing::_))
        .Times(2)
        .WillRepeatedly(::testing::Return(1));

    std::vector<Workout> workouts = {makePlannedWorkout("W1"), makePlannedWorkout("W2")};
    m_service->importPlannedWorkouts(workouts);
}

TEST_F(WorkoutServiceTest, ImportPlannedWorkouts_SavesEachWorkout)
{
    QStringList savedNames;

    EXPECT_CALL(m_repo, remove(::testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(m_repo, save(::testing::_))
        .Times(3)
        .WillRepeatedly([&savedNames](const Workout &w)
                        {
            savedNames.append(w.name());
            return savedNames.size(); });

    std::vector<Workout> workouts = {
        makePlannedWorkout("Push"), makePlannedWorkout("Pull"), makePlannedWorkout("Legs")};
    m_service->importPlannedWorkouts(workouts);

    EXPECT_EQ(savedNames.size(), 3);
    EXPECT_EQ(savedNames[0], "Push");
    EXPECT_EQ(savedNames[1], "Pull");
    EXPECT_EQ(savedNames[2], "Legs");
}

// --- removeAllPlannedWorkouts ---

TEST_F(WorkoutServiceTest, RemoveAllPlannedWorkouts_QueriesStartedTimeIsNull)
{
    EXPECT_CALL(m_repo, remove(::testing::_))
        .WillOnce([](const WorkoutQuery &query)
                  {
            EXPECT_TRUE(query.startedTimeIsNull().has_value());
            EXPECT_TRUE(query.startedTimeIsNull().value());
            return true; });

    m_service->removeAllPlannedWorkouts();
}

// --- loadHistory ---

TEST_F(WorkoutServiceTest, LoadHistory_QueriesStartedTimeIsNotNull)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce([](const WorkoutQuery &query)
                  {
            EXPECT_TRUE(query.startedTimeIsNull().has_value());
            EXPECT_FALSE(query.startedTimeIsNull().value());
            return std::vector<Workout>{}; });

    m_service->loadHistory();
}

TEST_F(WorkoutServiceTest, LoadHistory_OrdersByStartedTimeDescending)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce([](const WorkoutQuery &query)
                  {
            EXPECT_TRUE(query.orderByStartedTimeDirection().has_value());
            EXPECT_EQ(query.orderByStartedTimeDirection().value(), SortDirection::Descending);
            return std::vector<Workout>{}; });

    m_service->loadHistory();
}

TEST_F(WorkoutServiceTest, LoadHistory_ReturnsWorkoutsFromRepo)
{
    std::vector<Workout> history = {makeCompletedWorkout("Session 1"), makeCompletedWorkout("Session 2")};

    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce(::testing::Return(history));

    auto result = m_service->loadHistory();
    EXPECT_EQ(result.size(), 2u);
}

// --- findWorkout ---

TEST_F(WorkoutServiceTest, FindWorkout_QueriesById)
{
    Workout w = makePlannedWorkout("Found");
    w.setId(42);

    EXPECT_CALL(m_repo, findOne(::testing::_))
        .WillOnce([](const WorkoutQuery &query)
                  {
            EXPECT_TRUE(query.id().has_value());
            EXPECT_EQ(query.id().value(), 42);
            Workout found("Found", QDateTime::currentDateTime());
            found.setId(42);
            return std::optional<Workout>{found}; });

    auto result = m_service->findWorkout(42);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id(), 42);
}

TEST_F(WorkoutServiceTest, FindWorkout_ReturnsNulloptWhenNotFound)
{
    EXPECT_CALL(m_repo, findOne(::testing::_))
        .WillOnce(::testing::Return(std::nullopt));

    auto result = m_service->findWorkout(999);
    EXPECT_FALSE(result.has_value());
}

// --- saveWorkout ---

TEST_F(WorkoutServiceTest, SaveWorkout_DelegatesToRepo)
{
    Workout w = makePlannedWorkout("New Workout");

    EXPECT_CALL(m_repo, save(::testing::_))
        .WillOnce(::testing::Return(7));

    int id = m_service->saveWorkout(w);
    EXPECT_EQ(id, 7);
}

// --- deleteWorkout ---

TEST_F(WorkoutServiceTest, DeleteWorkout_QueriesById)
{
    EXPECT_CALL(m_repo, remove(::testing::_))
        .WillOnce([](const WorkoutQuery &query)
                  {
            EXPECT_TRUE(query.id().has_value());
            EXPECT_EQ(query.id().value(), 5);
            return true; });

    bool result = m_service->deleteWorkout(5);
    EXPECT_TRUE(result);
}

TEST_F(WorkoutServiceTest, DeleteWorkout_ReturnsFalseWhenNotFound)
{
    EXPECT_CALL(m_repo, remove(::testing::_))
        .WillOnce(::testing::Return(false));

    bool result = m_service->deleteWorkout(999);
    EXPECT_FALSE(result);
}
