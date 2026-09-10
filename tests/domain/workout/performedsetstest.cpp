#include "domain/workout/performedsets.h"
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
    Workout workout(QStringLiteral("Base Strength"), QDateTime(QDate(2025, 1, 2), QTime(18, 0, 0)));
    for (const Exercise& exercise : exercises)
        workout.addExercise(exercise);
    return workout;
}

}  // namespace

class PerformedSetsTest : public ::testing::Test
{
};

TEST_F(PerformedSetsTest, ASessionWithNoTicksAnywhereCarriesNoInformationInTheFlag)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0), Set(5, 45.0) }),
                        exerciseWith(QStringLiteral("Row"), { Set(8, 40.0) }) });

    EXPECT_FALSE(completionFlagsAreMeaningful(session));
}

TEST_F(PerformedSetsTest, ASingleTickAnywhereMakesTheFlagMeaningfulForTheWholeSession)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0) }),
                        exerciseWith(QStringLiteral("Row"), { tickedSet(8, 40.0) }) });

    EXPECT_TRUE(completionFlagsAreMeaningful(session));
}

TEST_F(PerformedSetsTest, AnEmptySessionHasNoMeaningfulFlags)
{
    EXPECT_FALSE(completionFlagsAreMeaningful(sessionWith({})));
}

TEST_F(PerformedSetsTest, WithoutMeaningfulFlagsEverySetWithContentCounts)
{
    const Exercise bench = exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0) });

    EXPECT_TRUE(wasPerformed(bench, false));
    EXPECT_FALSE(wasPerformed(bench, true));
}

TEST_F(PerformedSetsTest, AnExerciseWithoutSetsIsNeverPerformed)
{
    const Exercise empty = exerciseWith(QStringLiteral("Bench Press"), {});

    EXPECT_FALSE(wasPerformed(empty, false));
    EXPECT_FALSE(wasPerformed(empty, true));
}

TEST_F(PerformedSetsTest, WithMeaningfulFlagsOnlyTickedExercisesCount)
{
    const Exercise skipped = exerciseWith(QStringLiteral("Row"), { Set(8, 40.0) });
    const Exercise done = exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 45.0) });

    EXPECT_FALSE(wasPerformed(skipped, true));
    EXPECT_TRUE(wasPerformed(done, true));
}

TEST_F(PerformedSetsTest, WithoutMeaningfulFlagsEverySetIsReportedAsPerformed)
{
    const Exercise bench
        = exerciseWith(QStringLiteral("Bench Press"), { Set(5, 20.0), Set(5, 30.0), Set(5, 45.0) });

    const Exercise performed = asPerformed(bench, false);

    ASSERT_EQ(performed.sets().size(), 3u);
    for (const Set& set : performed.sets())
        EXPECT_TRUE(set.completed());
    EXPECT_DOUBLE_EQ(performed.sets()[2].weight(), 45.0);
}

TEST_F(PerformedSetsTest, WithMeaningfulFlagsTheExerciseIsReportedUntouched)
{
    const Exercise bench
        = exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 20.0), Set(5, 45.0) });

    const Exercise performed = asPerformed(bench, true);

    ASSERT_EQ(performed.sets().size(), 2u);
    EXPECT_TRUE(performed.sets()[0].completed());
    EXPECT_FALSE(performed.sets()[1].completed());
}

TEST_F(PerformedSetsTest, ReportingAsPerformedKeepsTheValues)
{
    const Exercise bench = exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0) });

    const Exercise performed = asPerformed(bench, false);

    ASSERT_EQ(performed.sets().size(), 1u);
    EXPECT_EQ(performed.sets()[0].repetitions(), 5);
    EXPECT_DOUBLE_EQ(performed.sets()[0].weight(), 45.0);
    EXPECT_EQ(performed.name(), QStringLiteral("Bench Press"));
}
