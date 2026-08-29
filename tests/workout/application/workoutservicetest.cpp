#include "modules/workout/application/workoutservice.h"
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/workoutstatus.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/domain/repositories/workoutrepository.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockWorkoutRepository : public WorkoutRepository
{
public:
    MOCK_METHOD(std::vector<Workout>, findAll, (const WorkoutQuery& query), (const, override));
    MOCK_METHOD(std::optional<Workout>, findOne, (const WorkoutQuery& query), (const, override));
    MOCK_METHOD(int, save, (const Workout& workout), (override));
    MOCK_METHOD(bool, remove, (const WorkoutQuery& query), (override));
    MOCK_METHOD(int, count, (const WorkoutQuery& query), (const, override));
    MOCK_METHOD(bool, exists, (const WorkoutQuery& query), (const, override));
};

class WorkoutServiceTest : public ::testing::Test
{
protected:
    void SetUp() override { m_service = std::make_unique<WorkoutService>(m_repo, nullptr); }

    std::vector<Workout> loadPlannedWorkouts()
    {
        return m_service->loadPlannedWorkoutsCore().value();
    }
    void importPlannedWorkouts(const std::vector<Workout>& workouts)
    {
        m_service->importPlannedWorkoutsCore(workouts);
    }
    void removeAllPlannedWorkouts() { m_service->removeAllPlannedWorkoutsCore(); }
    std::vector<Workout> loadHistory(int limit = -1)
    {
        return m_service->loadHistoryCore(limit).value();
    }
    std::vector<WorkoutService::ExerciseFrequency> topExercises(int topN, int recentWorkouts)
    {
        return m_service->topExercisesCore(topN, recentWorkouts).value();
    }
    WorkoutService::TrainingTotals recentTotals(int recentWorkouts)
    {
        return m_service->recentTotalsCore(recentWorkouts).value();
    }
    std::optional<Workout> findWorkout(int id) { return m_service->findWorkoutCore(id).value(); }
    int saveWorkout(const Workout& workout) { return m_service->saveWorkoutCore(workout).value(); }
    bool deleteWorkout(int id) { return m_service->deleteWorkoutCore(id).value(); }

    Workout makePlannedWorkout(const QString& name, int daysFromNow = 1)
    {
        Workout w(name, QDateTime::currentDateTime());
        w.setPlannedTime(QDateTime::currentDateTime().addDays(daysFromNow));
        return w;
    }

    Workout makeStartedWorkout(const QString& name)
    {
        Workout w(name, QDateTime::currentDateTime());
        w.setStartedTime(QDateTime::currentDateTime());
        return w;
    }

    Workout makeCompletedWorkout(const QString& name)
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

TEST_F(WorkoutServiceTest, LoadPlannedWorkouts_QueriesStatusPlanned)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.status().has_value());
                EXPECT_EQ(query.status().value(), WorkoutStatus::Planned);
                return std::vector<Workout> {};
            });

    loadPlannedWorkouts();
}

TEST_F(WorkoutServiceTest, LoadPlannedWorkouts_OrdersByPlannedTimeAscending)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.orderByPlannedTimeDirection().has_value());
                EXPECT_EQ(query.orderByPlannedTimeDirection().value(), SortDirection::Ascending);
                return std::vector<Workout> {};
            });

    loadPlannedWorkouts();
}

TEST_F(WorkoutServiceTest, LoadPlannedWorkouts_ReturnsWorkoutsFromRepo)
{
    std::vector<Workout> planned = { makePlannedWorkout("Day A"), makePlannedWorkout("Day B") };

    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(planned));

    auto result = loadPlannedWorkouts();
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].name(), "Day A");
    EXPECT_EQ(result[1].name(), "Day B");
}

// --- importPlannedWorkouts ---

TEST_F(WorkoutServiceTest, ImportPlannedWorkouts_RemovesExistingFirst)
{
    ::testing::InSequence seq;

    EXPECT_CALL(m_repo, remove(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.status().has_value());
                EXPECT_EQ(query.status().value(), WorkoutStatus::Planned);
                return true;
            });

    EXPECT_CALL(m_repo, save(::testing::_)).Times(2).WillRepeatedly(::testing::Return(1));

    std::vector<Workout> workouts = { makePlannedWorkout("W1"), makePlannedWorkout("W2") };
    importPlannedWorkouts(workouts);
}

TEST_F(WorkoutServiceTest, ImportPlannedWorkouts_SavesEachWorkout)
{
    QStringList savedNames;

    EXPECT_CALL(m_repo, remove(::testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(m_repo, save(::testing::_))
        .Times(3)
        .WillRepeatedly(
            [&savedNames](const Workout& w)
            {
                savedNames.append(w.name());
                return savedNames.size();
            });

    std::vector<Workout> workouts
        = { makePlannedWorkout("Push"), makePlannedWorkout("Pull"), makePlannedWorkout("Legs") };
    importPlannedWorkouts(workouts);

    EXPECT_EQ(savedNames.size(), 3);
    EXPECT_EQ(savedNames[0], "Push");
    EXPECT_EQ(savedNames[1], "Pull");
    EXPECT_EQ(savedNames[2], "Legs");
}

// --- removeAllPlannedWorkouts ---

TEST_F(WorkoutServiceTest, RemoveAllPlannedWorkouts_QueriesStatusPlanned)
{
    EXPECT_CALL(m_repo, remove(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.status().has_value());
                EXPECT_EQ(query.status().value(), WorkoutStatus::Planned);
                return true;
            });

    removeAllPlannedWorkouts();
}

// --- loadHistory ---

TEST_F(WorkoutServiceTest, LoadHistory_QueriesStatusEnded)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.status().has_value());
                EXPECT_EQ(query.status().value(), WorkoutStatus::Ended);
                return std::vector<Workout> {};
            });

    loadHistory();
}

TEST_F(WorkoutServiceTest, LoadHistory_OrdersByStartedTimeDescending)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.orderByStartedTimeDirection().has_value());
                EXPECT_EQ(query.orderByStartedTimeDirection().value(), SortDirection::Descending);
                return std::vector<Workout> {};
            });

    loadHistory();
}

TEST_F(WorkoutServiceTest, LoadHistory_ReturnsWorkoutsFromRepo)
{
    std::vector<Workout> history
        = { makeCompletedWorkout("Session 1"), makeCompletedWorkout("Session 2") };

    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    auto result = loadHistory();
    EXPECT_EQ(result.size(), 2u);
}

// --- topExercises ---

namespace
{
Workout makeWorkoutWithExercise(const QString& exerciseName, int reps, double weight)
{
    Workout w(exerciseName, QDateTime::currentDateTime());
    Exercise e(exerciseName, 90);
    e.addSet(Set(reps, weight));
    w.addExercise(e);
    return w;
}
}  // namespace

TEST_F(WorkoutServiceTest, TopExercises_RanksByFrequencyThenBestOneRepMax)
{
    std::vector<Workout> history = {
        makeWorkoutWithExercise("Bench", 5, 100),  // 1RM 112.5
        makeWorkoutWithExercise("Bench", 5, 80),   // 1RM 90
        makeWorkoutWithExercise("Squat", 5, 120),  // 1RM 135
        makeWorkoutWithExercise("Deadlift", 5, 140),  // 1RM 157.5
    };

    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    auto result = topExercises(2, 20);

    // Bench wins on frequency; the Squat/Deadlift tie is broken by best 1RM.
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].name, "Bench");
    EXPECT_EQ(result[0].count, 2);
    EXPECT_DOUBLE_EQ(result[0].bestOneRepMax, 112.5);
    EXPECT_EQ(result[1].name, "Deadlift");
    EXPECT_EQ(result[1].count, 1);
    EXPECT_DOUBLE_EQ(result[1].bestOneRepMax, 157.5);
}

TEST_F(WorkoutServiceTest, TopExercises_LimitsHistoryQueryToRecentWorkouts)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.limit().has_value());
                EXPECT_EQ(query.limit().value(), 5);
                return std::vector<Workout> {};
            });

    topExercises(2, 5);
}

TEST_F(WorkoutServiceTest, TopExercises_EmptyHistory_ReturnsEmpty)
{
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(std::vector<Workout> {}));

    auto result = topExercises(2, 20);
    EXPECT_TRUE(result.empty());
}

// --- topExercises: non-weighted work has no 1RM to rank ---

namespace
{
Workout makeWorkoutWithTimedExercise(const QString& exerciseName, int seconds)
{
    Workout w(exerciseName, QDateTime::currentDateTime());
    Exercise e(exerciseName, 60);
    e.setKind(ExerciseKind::Interval);
    e.addSet(Set::createDuration(seconds));
    w.addExercise(e);
    return w;
}

Workout makeWorkoutWithDistanceExercise(const QString& exerciseName, double meters, int seconds)
{
    Workout w(exerciseName, QDateTime::currentDateTime());
    Exercise e(exerciseName, 0);
    e.setKind(ExerciseKind::Cardio);
    e.addSet(Set::createDistance(meters, seconds));
    w.addExercise(e);
    return w;
}

Workout completedWorkout(Workout workout)
{
    for (auto& exercise : workout.exercises())
        for (auto& set : exercise.sets())
            set.setCompleted(true);
    return workout;
}

Workout makeWorkoutWithBodyweightExercise(const QString& exerciseName, int reps)
{
    Workout w(exerciseName, QDateTime::currentDateTime());
    Exercise e(exerciseName, 90);
    e.setKind(ExerciseKind::Bodyweight);
    Set set(reps, 0.0);
    set.setLoadType(LoadType::Bodyweight);
    e.addSet(set);
    w.addExercise(e);
    return w;
}
}  // namespace

TEST_F(WorkoutServiceTest, TopExercises_SkipsTimedAndDistanceExercises)
{
    std::vector<Workout> history = {
        makeWorkoutWithTimedExercise("Plank", 45),
        makeWorkoutWithTimedExercise("Plank", 45),
        makeWorkoutWithTimedExercise("Plank", 45),
        makeWorkoutWithDistanceExercise("Run", 5000, 1440),
        makeWorkoutWithExercise("Bench", 5, 100),
    };

    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    auto result = topExercises(2, 20);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].name, "Bench");
}

TEST_F(WorkoutServiceTest, TopExercises_SkipsBodyweightExercises)
{
    std::vector<Workout> history = {
        makeWorkoutWithBodyweightExercise("Pull-ups", 8),
        makeWorkoutWithExercise("Bench", 5, 100),
    };

    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    auto result = topExercises(2, 20);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].name, "Bench");
}

// --- recentTotals ---

TEST_F(WorkoutServiceTest, RecentTotals_SumsTimeDistanceAndVolume)
{
    std::vector<Workout> history = {
        completedWorkout(makeWorkoutWithTimedExercise("Plank", 45)),
        completedWorkout(makeWorkoutWithTimedExercise("Plank", 75)),
        completedWorkout(makeWorkoutWithDistanceExercise("Run", 5000, 1440)),
        completedWorkout(makeWorkoutWithExercise("Bench", 5, 100)),
    };

    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    const auto totals = recentTotals(20);

    EXPECT_EQ(totals.workouts, 4);
    EXPECT_EQ(totals.totalDurationSeconds, 45 + 75 + 1440);
    EXPECT_DOUBLE_EQ(totals.totalDistanceMeters, 5000.0);
    EXPECT_DOUBLE_EQ(totals.totalWeight, 500.0);
}

TEST_F(WorkoutServiceTest, RecentTotals_IgnoresSetsTheUserNeverDid)
{
    std::vector<Workout> history = {
        completedWorkout(makeWorkoutWithTimedExercise("Plank", 45)),
        makeWorkoutWithTimedExercise("Plank", 45),
        makeWorkoutWithDistanceExercise("Run", 5000, 1440),
    };

    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    const auto totals = recentTotals(20);

    EXPECT_EQ(totals.workouts, 3);
    EXPECT_EQ(totals.totalDurationSeconds, 45);
    EXPECT_DOUBLE_EQ(totals.totalDistanceMeters, 0.0);
}

TEST_F(WorkoutServiceTest, RecentTotals_LimitsHistoryQueryToRecentWorkouts)
{
    EXPECT_CALL(m_repo, findAll(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.limit().has_value());
                EXPECT_EQ(query.limit().value(), 5);
                return std::vector<Workout> {};
            });

    recentTotals(5);
}

TEST_F(WorkoutServiceTest, RecentTotals_EmptyHistory_IsAllZero)
{
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(std::vector<Workout> {}));

    const auto totals = recentTotals(20);

    EXPECT_EQ(totals.workouts, 0);
    EXPECT_EQ(totals.totalDurationSeconds, 0);
    EXPECT_DOUBLE_EQ(totals.totalDistanceMeters, 0.0);
    EXPECT_DOUBLE_EQ(totals.totalWeight, 0.0);
}

// --- findWorkout ---

TEST_F(WorkoutServiceTest, FindWorkout_QueriesById)
{
    Workout w = makePlannedWorkout("Found");
    w.setId(42);

    EXPECT_CALL(m_repo, findOne(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.id().has_value());
                EXPECT_EQ(query.id().value(), 42);
                Workout found("Found", QDateTime::currentDateTime());
                found.setId(42);
                return std::optional<Workout> { found };
            });

    auto result = findWorkout(42);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id(), 42);
}

TEST_F(WorkoutServiceTest, FindWorkout_ReturnsNulloptWhenNotFound)
{
    EXPECT_CALL(m_repo, findOne(::testing::_)).WillOnce(::testing::Return(std::nullopt));

    auto result = findWorkout(999);
    EXPECT_FALSE(result.has_value());
}

// --- saveWorkout ---

TEST_F(WorkoutServiceTest, SaveWorkout_DelegatesToRepo)
{
    Workout w = makePlannedWorkout("New Workout");

    EXPECT_CALL(m_repo, save(::testing::_)).WillOnce(::testing::Return(7));

    int id = saveWorkout(w);
    EXPECT_EQ(id, 7);
}

// --- deleteWorkout ---

TEST_F(WorkoutServiceTest, DeleteWorkout_QueriesById)
{
    EXPECT_CALL(m_repo, remove(::testing::_))
        .WillOnce(
            [](const WorkoutQuery& query)
            {
                EXPECT_TRUE(query.id().has_value());
                EXPECT_EQ(query.id().value(), 5);
                return true;
            });

    bool result = deleteWorkout(5);
    EXPECT_TRUE(result);
}

TEST_F(WorkoutServiceTest, DeleteWorkout_ReturnsFalseWhenNotFound)
{
    EXPECT_CALL(m_repo, remove(::testing::_)).WillOnce(::testing::Return(false));

    bool result = deleteWorkout(999);
    EXPECT_FALSE(result);
}
