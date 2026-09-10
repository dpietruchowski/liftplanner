#include "domain/workout/setnotation.h"
#include "domain/workout/exercise.h"

#include <gtest/gtest.h>

namespace
{

Set liftedSet(int repetitions, double weight, bool completed)
{
    Set set(repetitions, weight);
    set.setCompleted(completed);
    return set;
}

Exercise benchPress(std::initializer_list<Set> sets)
{
    Exercise exercise(QStringLiteral("Bench Press"), 120);
    for (const Set& set : sets)
        exercise.addSet(set);
    return exercise;
}

}

TEST(SetNotationTest, AMarkerIsAppendedOnlyToADoneSet)
{
    EXPECT_EQ(SetNotation::marked(QStringLiteral("10x80kg"), true), QStringLiteral("10x80kg!"));
    EXPECT_EQ(SetNotation::marked(QStringLiteral("10x80kg"), false), QStringLiteral("10x80kg"));
}

TEST(SetNotationTest, TakingTheMarkerStripsItAndReportsIt)
{
    QString token = QStringLiteral("10x80kg!");

    EXPECT_TRUE(SetNotation::takeCompletedMarker(token));
    EXPECT_EQ(token, QStringLiteral("10x80kg"));
}

TEST(SetNotationTest, AnUnmarkedTokenIsLeftAlone)
{
    QString token = QStringLiteral("10x80kg");

    EXPECT_FALSE(SetNotation::takeCompletedMarker(token));
    EXPECT_EQ(token, QStringLiteral("10x80kg"));
}

TEST(SetNotationTest, ThePlainSetsTextNeverShowsTheMarker)
{
    const Exercise exercise = benchPress({ liftedSet(5, 40.0, true), liftedSet(5, 40.0, false) });

    EXPECT_EQ(exercise.setsToString(), QStringLiteral("5x40kg, 5x40kg"));
}

TEST(SetNotationTest, OnlyTheTickedSetCarriesTheMarker)
{
    const Exercise exercise = benchPress({ liftedSet(5, 40.0, true), liftedSet(5, 40.0, false) });

    EXPECT_EQ(exercise.setsToStringMarkingCompleted(), QStringLiteral("5x40kg!, 5x40kg"));
}

TEST(SetNotationTest, ASessionWithoutASingleTickIsWrittenWithoutMarkers)
{
    const Exercise exercise = benchPress({ liftedSet(5, 40.0, false), liftedSet(5, 40.0, false) });

    EXPECT_EQ(exercise.setsToStringMarkingCompleted(), QStringLiteral("5x40kg, 5x40kg"));
}

TEST(SetNotationTest, EverySetTickedMarksEveryToken)
{
    const Exercise exercise = benchPress({ liftedSet(5, 40.0, true), liftedSet(3, 50.0, true) });

    EXPECT_EQ(exercise.setsToStringMarkingCompleted(), QStringLiteral("5x40kg!, 3x50kg!"));
}

TEST(SetNotationTest, ARepeatedRunWithARestOverrideCarriesOneMarker)
{
    Exercise exercise(QStringLiteral("Burpees"), 60);
    for (int i = 0; i < 3; ++i)
    {
        Set work = Set::createDuration(20);
        work.setRestSecondsOverride(10);
        work.setCompleted(true);
        exercise.addSet(work);
    }

    EXPECT_EQ(exercise.setsToStringMarkingCompleted(), QStringLiteral("3x(20s/10s)!"));
}

TEST(SetNotationTest, ARunBreaksApartWhereTheTicksStop)
{
    Exercise exercise(QStringLiteral("Burpees"), 60);
    for (int i = 0; i < 3; ++i)
    {
        Set work = Set::createDuration(20);
        work.setRestSecondsOverride(10);
        work.setCompleted(i < 2);
        exercise.addSet(work);
    }

    EXPECT_EQ(exercise.setsToStringMarkingCompleted(), QStringLiteral("2x(20s/10s)!, 1x(20s/10s)"));
}
