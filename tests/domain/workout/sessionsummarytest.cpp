#include "domain/workout/sessionsummary.h"
#include <gtest/gtest.h>

namespace
{

Set tickedSet(int repetitions, double weight)
{
    Set set(repetitions, weight);
    set.setCompleted(true);
    return set;
}

Exercise exerciseWith(const QString& name, std::initializer_list<Set> sets)
{
    Exercise exercise(name, 120);
    for (const Set& set : sets)
        exercise.addSet(set);
    return exercise;
}

Workout sessionWith(std::initializer_list<Exercise> exercises)
{
    Workout workout(QStringLiteral("Push Day"), QDateTime(QDate(2025, 3, 4), QTime(17, 0, 0)));
    for (const Exercise& exercise : exercises)
        workout.addExercise(exercise);
    workout.setStartedTime(QDateTime(QDate(2025, 3, 4), QTime(17, 0, 0)));
    workout.setEndedTime(QDateTime(QDate(2025, 3, 4), QTime(18, 5, 0)));
    return workout;
}

}  // namespace

class SessionSummaryTest : public ::testing::Test
{
};

TEST_F(SessionSummaryTest, AFullyTickedSessionReportsEverySetAndItsWholeVolume)
{
    const Workout session = sessionWith(
        { exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 60.0), tickedSet(5, 60.0) }),
          exerciseWith(QStringLiteral("Row"), { tickedSet(10, 40.0) }) });

    const SessionSummary summary = summarizeSession(session);

    EXPECT_EQ(summary.completedSets, 3);
    EXPECT_EQ(summary.plannedSets, 3);
    EXPECT_DOUBLE_EQ(summary.volume, 1000.0);
}

TEST_F(SessionSummaryTest, AnAbandonedSessionReportsWhatWasDoneNotWhatWasPlanned)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"),
                                     { tickedSet(5, 60.0), Set(5, 60.0), Set(5, 60.0) }),
                        exerciseWith(QStringLiteral("Row"), { Set(10, 40.0), Set(10, 40.0) }) });

    const SessionSummary summary = summarizeSession(session);

    EXPECT_EQ(summary.completedSets, 1);
    EXPECT_EQ(summary.plannedSets, 5);
    EXPECT_DOUBLE_EQ(summary.volume, 300.0);
}

TEST_F(SessionSummaryTest, AnUntouchedExerciseAddsNothingToAnAbandonedSession)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Squat"), { tickedSet(5, 100.0) }),
                        exerciseWith(QStringLiteral("Deadlift"), { Set(5, 140.0) }) });

    const SessionSummary summary = summarizeSession(session);

    EXPECT_EQ(summary.completedSets, 1);
    EXPECT_DOUBLE_EQ(summary.volume, 500.0);
}

TEST_F(SessionSummaryTest, ASessionWithoutASingleTickFallsBackToTheSharedRule)
{
    const Workout session = sessionWith(
        { exerciseWith(QStringLiteral("Bench Press"), { Set(5, 60.0), Set(5, 60.0) }) });

    const SessionSummary summary = summarizeSession(session);

    EXPECT_EQ(summary.completedSets, 2);
    EXPECT_DOUBLE_EQ(summary.volume, 600.0);
}

TEST_F(SessionSummaryTest, TimedWorkCountsAsASetButCarriesNoVolume)
{
    Set plank = Set::createDuration(60);
    plank.setCompleted(true);

    const Workout session = sessionWith({ exerciseWith(QStringLiteral("Plank"), { plank }) });

    const SessionSummary summary = summarizeSession(session);

    EXPECT_EQ(summary.completedSets, 1);
    EXPECT_DOUBLE_EQ(summary.volume, 0.0);
}

TEST_F(SessionSummaryTest, BodyweightRepsCountAsSetsButCarryNoVolume)
{
    Set pullUp = tickedSet(8, 0.0);
    pullUp.setLoadType(LoadType::Bodyweight);

    const Workout session = sessionWith({ exerciseWith(QStringLiteral("Pull Up"), { pullUp }) });

    const SessionSummary summary = summarizeSession(session);

    EXPECT_EQ(summary.completedSets, 1);
    EXPECT_DOUBLE_EQ(summary.volume, 0.0);
}

TEST_F(SessionSummaryTest, TheDurationIsTheWallClockSpanOfTheSession)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Squat"), { tickedSet(5, 100.0) }) });

    EXPECT_EQ(summarizeSession(session).durationSeconds, 3900);
}

TEST_F(SessionSummaryTest, ASessionThatNeverEndedReportsNoDuration)
{
    Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Squat"), { tickedSet(5, 100.0) }) });
    session.setEndedTime(QDateTime());

    EXPECT_EQ(summarizeSession(session).durationSeconds, 0);
}

TEST_F(SessionSummaryTest, AnEmptySessionHasNothingToSummarize)
{
    const SessionSummary summary = summarizeSession(sessionWith({}));

    EXPECT_TRUE(summary.isEmpty());
    EXPECT_EQ(summary.plannedSets, 0);
    EXPECT_DOUBLE_EQ(summary.volume, 0.0);
}
