#include "ui/presentation/workouttext.h"
#include "domain/workout/performedsets.h"
#include <gtest/gtest.h>

class WorkoutTextTest : public ::testing::Test
{
};

namespace
{

Set tickedSet(int repetitions, double weight)
{
    Set set(repetitions, weight);
    set.setCompleted(true);
    return set;
}

Set completedCopy(Set set)
{
    set.setCompleted(true);
    return set;
}

Exercise sessionOf(std::initializer_list<Set> sets)
{
    Exercise exercise(QStringLiteral("Back Squat"), 180);
    for (const Set& set : sets)
        exercise.addSet(set);
    return exercise;
}

}

TEST_F(WorkoutTextTest, AbandonPrompt_PromisesThePlannedListWhenExercisesWereAdded)
{
    const QString prompt = WorkoutText::abandonPrompt(2);

    EXPECT_TRUE(prompt.contains(QStringLiteral("planned list")));
    EXPECT_FALSE(prompt.contains(QStringLiteral("for good")));
}

TEST_F(WorkoutTextTest, AbandonPrompt_SaysUpFrontThatAnEmptySessionDisappears)
{
    const QString prompt = WorkoutText::abandonPrompt(0);

    EXPECT_TRUE(prompt.contains(QStringLiteral("for good")));
    EXPECT_TRUE(prompt.contains(QStringLiteral("history")));
    EXPECT_TRUE(prompt.contains(QStringLiteral("planned list")));
}

TEST_F(WorkoutTextTest, FormatDuration_DropsToSecondsBelowAMinute)
{
    EXPECT_EQ(WorkoutText::formatDuration(0), "0s");
    EXPECT_EQ(WorkoutText::formatDuration(45), "45s");
}

TEST_F(WorkoutTextTest, FormatDuration_UsesMinutesThenHours)
{
    EXPECT_EQ(WorkoutText::formatDuration(60), "1m");
    EXPECT_EQ(WorkoutText::formatDuration(1560), "26m");
    EXPECT_EQ(WorkoutText::formatDuration(3600), "1h 00m");
    EXPECT_EQ(WorkoutText::formatDuration(4500), "1h 15m");
}

TEST_F(WorkoutTextTest, FormatRest_ReadsAsMinutesAndPaddedSeconds)
{
    EXPECT_EQ(WorkoutText::formatRest(120), "2:00");
    EXPECT_EQ(WorkoutText::formatRest(90), "1:30");
    EXPECT_EQ(WorkoutText::formatRest(45), "0:45");
    EXPECT_EQ(WorkoutText::formatRest(605), "10:05");
}

TEST_F(WorkoutTextTest, FormatRest_TreatsMissingRestAsZero)
{
    EXPECT_EQ(WorkoutText::formatRest(0), "0:00");
    EXPECT_EQ(WorkoutText::formatRest(-30), "0:00");
}

TEST_F(WorkoutTextTest, FormatDistance_UsesMetresBelowAKilometre)
{
    EXPECT_EQ(WorkoutText::formatDistance(0.0), "0 m");
    EXPECT_EQ(WorkoutText::formatDistance(400.0), "400 m");
    EXPECT_EQ(WorkoutText::formatDistance(999.4), "999 m");
}

TEST_F(WorkoutTextTest, FormatDistance_SwitchesToKilometresAndTrimsTheZeroDecimal)
{
    EXPECT_EQ(WorkoutText::formatDistance(1000.0), "1 km");
    EXPECT_EQ(WorkoutText::formatDistance(5000.0), "5 km");
    EXPECT_EQ(WorkoutText::formatDistance(12400.0), "12.4 km");
}

TEST_F(WorkoutTextTest, FormatVolume_StaysInKilogramsAndRoundsToWholeOnes)
{
    EXPECT_EQ(WorkoutText::formatVolume(0.0), "0 kg");
    EXPECT_EQ(WorkoutText::formatVolume(900.0), "900 kg");
    EXPECT_EQ(WorkoutText::formatVolume(987.5), "988 kg");
}

TEST_F(WorkoutTextTest, FormatVolume_GroupsThousandsSoBigNumbersStayReadable)
{
    EXPECT_EQ(WorkoutText::formatVolume(1000.0), "1 000 kg");
    EXPECT_EQ(WorkoutText::formatVolume(12400.0), "12 400 kg");
    EXPECT_EQ(WorkoutText::formatVolume(203450.0), "203 450 kg");
    EXPECT_EQ(WorkoutText::formatVolume(1234567.0), "1 234 567 kg");
}

TEST_F(WorkoutTextTest, FormatVolume_NeverReportsNegativeWork)
{
    EXPECT_EQ(WorkoutText::formatVolume(-50.0), "0 kg");
}

TEST_F(WorkoutTextTest, PreviousSetHints_MatchSetsByTheirPosition)
{
    const Exercise previous
        = sessionOf({ tickedSet(5, 50.0), tickedSet(5, 60.0), tickedSet(3, 70.0) });

    const QStringList hints = WorkoutText::previousSetHints(previous, 3);

    ASSERT_EQ(hints.size(), 3);
    EXPECT_EQ(hints[0], "5x50kg");
    EXPECT_EQ(hints[1], "5x60kg");
    EXPECT_EQ(hints[2], "3x70kg");
}

TEST_F(WorkoutTextTest, PreviousSetHints_LeaveSetsBeyondTheLastSessionEmpty)
{
    const Exercise previous = sessionOf({ tickedSet(5, 50.0), tickedSet(5, 60.0) });

    const QStringList hints = WorkoutText::previousSetHints(previous, 4);

    ASSERT_EQ(hints.size(), 4);
    EXPECT_EQ(hints[1], "5x60kg");
    EXPECT_TRUE(hints[2].isEmpty());
    EXPECT_TRUE(hints[3].isEmpty());
}

TEST_F(WorkoutTextTest, PreviousSetHints_StopAtTodaysSetCount)
{
    const Exercise previous
        = sessionOf({ tickedSet(5, 50.0), tickedSet(5, 60.0), tickedSet(5, 70.0) });

    EXPECT_EQ(WorkoutText::previousSetHints(previous, 1).size(), 1);
    EXPECT_TRUE(WorkoutText::previousSetHints(previous, 0).isEmpty());
}

TEST_F(WorkoutTextTest, PreviousSetHints_SkipSetsThatWereNotDone)
{
    const Exercise previous = sessionOf({ tickedSet(5, 50.0), Set(5, 60.0), tickedSet(3, 70.0) });

    const QStringList hints = WorkoutText::previousSetHints(previous, 3);

    ASSERT_EQ(hints.size(), 3);
    EXPECT_EQ(hints[0], "5x50kg");
    EXPECT_EQ(hints[1], "3x70kg");
    EXPECT_TRUE(hints[2].isEmpty());
}

TEST_F(WorkoutTextTest, PreviousSetHints_TakeAllSetsOfASessionWithoutASingleTick)
{
    const Exercise imported = sessionOf({ Set(8, 40.0), Set(8, 40.0), Set(8, 40.0) });

    const QStringList hints = WorkoutText::previousSetHints(asPerformed(imported, false), 3);

    ASSERT_EQ(hints.size(), 3);
    EXPECT_EQ(hints[0], "8x40kg");
    EXPECT_EQ(hints[2], "8x40kg");
}

TEST_F(WorkoutTextTest, PreviousSetHints_SpellOutBodyweightLoads)
{
    Set added(10, 22.5);
    added.setLoadType(LoadType::Added);
    Set assisted(10, 22.5);
    assisted.setLoadType(LoadType::Assisted);

    const Exercise previous = sessionOf({ completedCopy(added), completedCopy(assisted) });

    const QStringList hints = WorkoutText::previousSetHints(previous, 2);

    ASSERT_EQ(hints.size(), 2);
    EXPECT_EQ(hints[0], "10xBW+22.5kg");
    EXPECT_EQ(hints[1], "10xBW-22.5kg");
}

TEST_F(WorkoutTextTest, PreviousSetHints_AreAllEmptyForAnExerciseWithoutAPastSession)
{
    const QStringList hints = WorkoutText::previousSetHints(Exercise(), 2);

    ASSERT_EQ(hints.size(), 2);
    EXPECT_TRUE(hints[0].isEmpty());
    EXPECT_TRUE(hints[1].isEmpty());
}
