#include "application/workout/workoutservice.h"
#include "domain/workout/historysetrow.h"
#include "domain/workout/workout.h"
#include "domain/workout/workoutquery.h"
#include "domain/workout/workoutrepository.h"
#include "domain/workout/workoutrowrepository.h"
#include "domain/workout/workoutstatus.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockWorkoutRepository : public WorkoutRepository
{
public:
    MOCK_METHOD(std::vector<Workout>, findAll, (const WorkoutQuery& query), (const, override));
    MOCK_METHOD(std::optional<Workout>, findOne, (const WorkoutQuery& query), (const, override));
    MOCK_METHOD(int, save, (const Workout& workout), (override));
    MOCK_METHOD(bool, saveSet, (const Set& set), (override));
    MOCK_METHOD(bool, remove, (const WorkoutQuery& query), (override));
    MOCK_METHOD(int, count, (const WorkoutQuery& query), (const, override));
    MOCK_METHOD(bool, exists, (const WorkoutQuery& query), (const, override));
};

class MockWorkoutRowRepository : public WorkoutRowRepository
{
public:
    MOCK_METHOD(std::vector<HistorySetRow>, findSetsOfRecentWorkouts, (int workoutLimit),
                (const, override));
};

std::vector<HistorySetRow> rowsOf(const std::vector<Workout>& workouts)
{
    std::vector<HistorySetRow> rows;
    int workoutId = 0;
    int exerciseId = 0;

    for (const Workout& workout : workouts)
    {
        ++workoutId;
        if (workout.exercises().empty())
        {
            HistorySetRow row;
            row.workoutId = workoutId;
            rows.push_back(row);
            continue;
        }

        for (const Exercise& exercise : workout.exercises())
        {
            ++exerciseId;
            for (const Set& set : exercise.sets())
            {
                HistorySetRow row;
                row.workoutId = workoutId;
                row.exerciseId = exerciseId;
                row.exerciseName = exercise.name();
                row.hasSet = true;
                row.completed = set.completed();
                row.metric = set.metric();
                row.loadType = set.loadType();
                row.repetitions = set.repetitions();
                row.weight = set.weight();
                row.durationSeconds = set.durationSeconds();
                row.distanceMeters = set.distanceMeters();
                rows.push_back(row);
            }
        }
    }

    return rows;
}

class WorkoutServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_service = std::make_unique<WorkoutService>(m_repo, m_rowRepo, nullptr);
    }

    void givenHistoryRows(const std::vector<Workout>& workouts)
    {
        EXPECT_CALL(m_rowRepo, findSetsOfRecentWorkouts(::testing::_))
            .WillOnce(::testing::Return(rowsOf(workouts)));
    }

    void givenHistoryRowsRepeatedly(const std::vector<Workout>& workouts)
    {
        EXPECT_CALL(m_rowRepo, findSetsOfRecentWorkouts(::testing::_))
            .WillRepeatedly(::testing::Return(rowsOf(workouts)));
    }

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
    std::vector<WorkoutService::PreviousPerformance> previousPerformances(const Workout& workout)
    {
        return m_service->previousPerformancesCore(workout).value();
    }
    std::vector<WorkoutService::ExerciseSession> exerciseSessions(const Exercise& exercise,
                                                                  int limit)
    {
        return m_service->exerciseSessionsCore(exercise, limit).value();
    }
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
    MockWorkoutRowRepository m_rowRepo;
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
        makeWorkoutWithExercise("Bench", 5, 80),  // 1RM 90
        makeWorkoutWithExercise("Squat", 5, 120),  // 1RM 135
        makeWorkoutWithExercise("Deadlift", 5, 140),  // 1RM 157.5
    };

    givenHistoryRows(history);

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
    EXPECT_CALL(m_rowRepo, findSetsOfRecentWorkouts(5))
        .WillOnce(::testing::Return(std::vector<HistorySetRow> {}));

    topExercises(2, 5);
}

TEST_F(WorkoutServiceTest, TopExercises_EmptyHistory_ReturnsEmpty)
{
    givenHistoryRows({});

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

    givenHistoryRows(history);

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

    givenHistoryRows(history);

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

    givenHistoryRows(history);

    const auto totals = recentTotals(20);

    EXPECT_EQ(totals.workouts, 4);
    EXPECT_EQ(totals.totalDurationSeconds, 45 + 75 + 1440);
    EXPECT_DOUBLE_EQ(totals.totalDistanceMeters, 5000.0);
    EXPECT_DOUBLE_EQ(totals.totalWeight, 500.0);
}

namespace
{

Exercise weightedExercise(const QString& name, int reps, double weight, bool ticked)
{
    Exercise exercise(name, 90);
    Set set(reps, weight);
    set.setCompleted(ticked);
    exercise.addSet(set);
    return exercise;
}

Exercise timedExercise(const QString& name, int seconds, bool ticked)
{
    Exercise exercise(name, 60);
    exercise.setKind(ExerciseKind::Interval);
    Set set = Set::createDuration(seconds);
    set.setCompleted(ticked);
    exercise.addSet(set);
    return exercise;
}

Exercise distanceExercise(const QString& name, double meters, int seconds, bool ticked)
{
    Exercise exercise(name, 0);
    exercise.setKind(ExerciseKind::Cardio);
    Set set = Set::createDistance(meters, seconds);
    set.setCompleted(ticked);
    exercise.addSet(set);
    return exercise;
}

Workout sessionOf(const std::vector<Exercise>& exercises)
{
    Workout workout(QStringLiteral("Session"), QDateTime::currentDateTime());
    for (const Exercise& exercise : exercises)
        workout.addExercise(exercise);
    return workout;
}

}  // namespace

TEST_F(WorkoutServiceTest, RecentTotals_IgnoresUntickedSetsOfASessionThatWasTickedOff)
{
    std::vector<Workout> history = {
        sessionOf({ timedExercise("Plank", 45, true), timedExercise("Hollow Hold", 60, false) }),
    };

    givenHistoryRows(history);

    const auto totals = recentTotals(20);

    EXPECT_EQ(totals.workouts, 1);
    EXPECT_EQ(totals.totalDurationSeconds, 45);
}

TEST_F(WorkoutServiceTest, RecentTotals_CountsASessionThatWasNeverTickedOff)
{
    std::vector<Workout> history = {
        sessionOf({ timedExercise("Plank", 45, false) }),
        sessionOf({ distanceExercise("Run", 5000, 1440, false) }),
        sessionOf({ weightedExercise("Bench", 5, 100.0, false) }),
    };

    givenHistoryRows(history);

    const auto totals = recentTotals(20);

    EXPECT_EQ(totals.workouts, 3);
    EXPECT_EQ(totals.totalDurationSeconds, 45 + 1440);
    EXPECT_DOUBLE_EQ(totals.totalDistanceMeters, 5000.0);
    EXPECT_DOUBLE_EQ(totals.totalWeight, 500.0);
}

TEST_F(WorkoutServiceTest, TopExercises_IgnoresAnUntickedExerciseOfASessionThatWasTickedOff)
{
    std::vector<Workout> history = {
        sessionOf({ timedExercise("Plank", 45, true), weightedExercise("Bench", 5, 100.0, false) }),
    };

    givenHistoryRows(history);

    EXPECT_TRUE(topExercises(2, 20).empty());
}

TEST_F(WorkoutServiceTest, TopExercises_CountsASessionThatWasNeverTickedOff)
{
    std::vector<Workout> history = {
        sessionOf(
            { timedExercise("Plank", 45, false), weightedExercise("Bench", 5, 100.0, false) }),
    };

    givenHistoryRows(history);

    const auto result = topExercises(2, 20);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].name, "Bench");
    EXPECT_DOUBLE_EQ(result[0].bestOneRepMax, 112.5);
}

TEST_F(WorkoutServiceTest, HomeTiles_AgreeOnWhetherASessionCounts)
{
    struct Scenario
    {
        const char* description;
        bool benchTicked;
        bool companionTicked;
    };

    const std::vector<Scenario> scenarios = {
        { "nothing ticked anywhere", false, false },
        { "only the companion ticked", false, true },
        { "everything ticked", true, true },
        { "only the bench ticked", true, false },
    };

    for (const Scenario& scenario : scenarios)
    {
        givenHistoryRowsRepeatedly({
            sessionOf({ timedExercise("Plank", 45, scenario.companionTicked),
                        weightedExercise("Bench", 5, 100.0, scenario.benchTicked) }),
        });

        const bool benchIsAPersonalRecord = !topExercises(5, 20).empty();
        const bool benchAddsVolume = recentTotals(20).totalWeight > 0.0;

        EXPECT_EQ(benchIsAPersonalRecord, benchAddsVolume) << scenario.description;
    }
}

TEST_F(WorkoutServiceTest, RecentTotals_LimitsHistoryQueryToRecentWorkouts)
{
    EXPECT_CALL(m_rowRepo, findSetsOfRecentWorkouts(5))
        .WillOnce(::testing::Return(std::vector<HistorySetRow> {}));

    recentTotals(5);
}

TEST_F(WorkoutServiceTest, RecentTotals_EmptyHistory_IsAllZero)
{
    givenHistoryRows({});

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

// --- previousPerformances ---

namespace
{

Exercise exerciseWithSets(const QString& name, double weight, bool completed,
                          std::optional<int> definitionId = std::nullopt)
{
    Exercise exercise(name, 120);
    if (definitionId)
        exercise.setDefinitionId(*definitionId);
    for (int i = 0; i < 2; ++i)
    {
        Set set(10, weight);
        set.setCompleted(completed);
        exercise.addSet(set);
    }
    return exercise;
}

Workout endedWorkout(int id, int daysAgo, const std::vector<Exercise>& exercises)
{
    Workout workout(QStringLiteral("Session %1").arg(id), QDateTime::currentDateTime());
    workout.setId(id);
    workout.setStatus(WorkoutStatus::Ended);
    workout.setStartedTime(QDateTime::currentDateTime().addDays(-daysAgo));
    for (const Exercise& exercise : exercises)
        workout.addExercise(exercise);
    return workout;
}

}

TEST_F(WorkoutServiceTest, PreviousPerformances_SkipAnUntickedExerciseInATickedSession)
{
    const std::vector<Workout> history = {
        endedWorkout(4, 1,
                     { exerciseWithSets("Bench Press", 100.0, false, 7),
                       exerciseWithSets("Squat", 90.0, true) }),
        endedWorkout(2, 2, { exerciseWithSets("Bench Press", 62.5, true, 7) }),
        endedWorkout(1, 3,
                     { exerciseWithSets("Bench Press", 60.0, true, 7),
                       exerciseWithSets("Squat", 90.0, true) }),
    };
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    Workout active("Today", QDateTime::currentDateTime());
    active.setId(9);
    active.addExercise(exerciseWithSets("Bench Press", 0.0, false, 7));
    active.addExercise(exerciseWithSets("squat", 0.0, false));
    active.addExercise(exerciseWithSets("Deadlift", 0.0, false));

    const auto result = previousPerformances(active);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].exerciseIndex, 0);
    EXPECT_EQ(result[0].performedAt, history[1].startedTime());
    EXPECT_DOUBLE_EQ(result[0].exercise.sets()[0].weight(), 62.5);
    EXPECT_EQ(result[1].exerciseIndex, 1);
    EXPECT_EQ(result[1].exercise.name(), "Squat");
}

TEST_F(WorkoutServiceTest, PreviousPerformances_TrustsAFinishedSessionThatWasNeverTickedOff)
{
    const std::vector<Workout> history = {
        endedWorkout(4, 1, { exerciseWithSets("Bench Press", 45.0, false, 7) }),
    };
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    Workout active("Today", QDateTime::currentDateTime());
    active.setId(9);
    active.addExercise(exerciseWithSets("Bench Press", 0.0, false, 7));

    const auto result = previousPerformances(active);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].exerciseIndex, 0);
    ASSERT_EQ(result[0].exercise.sets().size(), 2u);
    EXPECT_DOUBLE_EQ(result[0].exercise.sets()[0].weight(), 45.0);
    EXPECT_TRUE(result[0].exercise.sets()[0].completed());
    EXPECT_TRUE(result[0].exercise.sets()[1].completed());
}

TEST_F(WorkoutServiceTest, PreviousPerformances_IgnoreAnExerciseThatHasNoSetsAtAll)
{
    const std::vector<Workout> history = {
        endedWorkout(4, 1, { Exercise(QStringLiteral("Bench Press"), 120) }),
    };
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    Workout active("Today", QDateTime::currentDateTime());
    active.setId(9);
    active.addExercise(exerciseWithSets("Bench Press", 0.0, false));

    EXPECT_TRUE(previousPerformances(active).empty());
}

TEST_F(WorkoutServiceTest, PreviousPerformances_SkipsTheWorkoutItselfAndNameMismatches)
{
    const std::vector<Workout> history = {
        endedWorkout(9, 0, { exerciseWithSets("Bench Press", 100.0, true, 7) }),
        endedWorkout(1, 3, { exerciseWithSets("Incline Bench Press", 60.0, true, 8) }),
    };
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    Workout active("Today", QDateTime::currentDateTime());
    active.setId(9);
    active.addExercise(exerciseWithSets("Bench Press", 0.0, false, 7));

    EXPECT_TRUE(previousPerformances(active).empty());
}

// --- exerciseSessions ---

TEST_F(WorkoutServiceTest, ExerciseSessions_ListsSeveralSessionsNewestFirst)
{
    const std::vector<Workout> history = {
        endedWorkout(5, 1, { exerciseWithSets("Back Squat", 100.0, true, 7) }),
        endedWorkout(4, 4, { exerciseWithSets("Back Squat", 95.0, true, 7) }),
        endedWorkout(3, 9, { exerciseWithSets("Back Squat", 90.0, true, 7) }),
    };
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    const auto result = exerciseSessions(exerciseWithSets("Back Squat", 0.0, false, 7), 5);

    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0].performedAt, history[0].startedTime());
    EXPECT_EQ(result[1].performedAt, history[1].startedTime());
    EXPECT_EQ(result[2].performedAt, history[2].startedTime());
    EXPECT_DOUBLE_EQ(result[0].exercise.sets()[0].weight(), 100.0);
    EXPECT_DOUBLE_EQ(result[2].exercise.sets()[0].weight(), 90.0);
}

TEST_F(WorkoutServiceTest, ExerciseSessions_StopsAtTheLimit)
{
    std::vector<Workout> history;
    for (int i = 0; i < 12; ++i)
        history.push_back(
            endedWorkout(100 + i, i, { exerciseWithSets("Back Squat", 100.0, true) }));
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    const auto result = exerciseSessions(exerciseWithSets("Back Squat", 0.0, false), 5);

    ASSERT_EQ(result.size(), 5u);
    EXPECT_EQ(result.back().performedAt, history[4].startedTime());
}

TEST_F(WorkoutServiceTest, ExerciseSessions_CountsASessionThatWasNeverTickedOff)
{
    const std::vector<Workout> history = {
        endedWorkout(5, 1, { exerciseWithSets("Back Squat", 40.0, false, 7) }),
    };
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    const auto result = exerciseSessions(exerciseWithSets("Back Squat", 0.0, false, 7), 5);

    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0].exercise.sets().size(), 2u);
    EXPECT_TRUE(result[0].exercise.sets()[0].completed());
    EXPECT_TRUE(result[0].exercise.sets()[1].completed());
    EXPECT_DOUBLE_EQ(result[0].exercise.sets()[0].weight(), 40.0);
}

TEST_F(WorkoutServiceTest, ExerciseSessions_SkipsAnUntickedExerciseInATickedSession)
{
    const std::vector<Workout> history = {
        endedWorkout(5, 1,
                     { exerciseWithSets("Back Squat", 120.0, false, 7),
                       exerciseWithSets("Bench Press", 80.0, true, 8) }),
        endedWorkout(4, 4, { exerciseWithSets("Back Squat", 95.0, true, 7) }),
    };
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    const auto result = exerciseSessions(exerciseWithSets("Back Squat", 0.0, false, 7), 5);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].performedAt, history[1].startedTime());
    EXPECT_DOUBLE_EQ(result[0].exercise.sets()[0].weight(), 95.0);
}

TEST_F(WorkoutServiceTest, ExerciseSessions_AreEmptyForAnExerciseDoneForTheFirstTime)
{
    const std::vector<Workout> history = {
        endedWorkout(5, 1, { exerciseWithSets("Bench Press", 80.0, true, 8) }),
    };
    EXPECT_CALL(m_repo, findAll(::testing::_)).WillOnce(::testing::Return(history));

    EXPECT_TRUE(exerciseSessions(exerciseWithSets("Back Squat", 0.0, false, 7), 5).empty());
}
