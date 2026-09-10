#include "domain/workout/setseeding.h"
#include <gtest/gtest.h>

namespace
{

Set completedSet(int repetitions, double weight)
{
    Set set(repetitions, weight);
    set.setCompleted(true);
    return set;
}

Set plannedSet(int repetitions, double weight) { return Set(repetitions, weight); }

Exercise benchWith(std::initializer_list<Set> sets)
{
    Exercise exercise(QStringLiteral("Bench Press"), 120);
    for (const Set& set : sets)
        exercise.addSet(set);
    return exercise;
}

}  // namespace

class SetSeedingTest : public ::testing::Test
{
};

TEST_F(SetSeedingTest, LastCompletedSetIsTheOneTheLifterFinishedOn)
{
    const Exercise exercise
        = benchWith({ completedSet(10, 60.0), completedSet(8, 70.0), plannedSet(8, 80.0) });

    const std::optional<Set> found = lastCompletedSet(exercise);

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->repetitions(), 8);
    EXPECT_DOUBLE_EQ(found->weight(), 70.0);
}

TEST_F(SetSeedingTest, AnExerciseWithoutACompletedSetHasNothingToOffer)
{
    const Exercise exercise = benchWith({ plannedSet(10, 60.0), plannedSet(10, 60.0) });

    EXPECT_FALSE(lastCompletedSet(exercise).has_value());
    EXPECT_FALSE(seedFromPreviousExercise(exercise, plannedSet(8, 0.0)).has_value());
}

TEST_F(SetSeedingTest, TheSeedTakesRepetitionsAndWeightFromTheLastTime)
{
    const Exercise previous = benchWith({ completedSet(10, 60.0), completedSet(5, 82.5) });

    const std::optional<Set> seeded = seedFromPreviousExercise(previous, plannedSet(8, 0.0));

    ASSERT_TRUE(seeded.has_value());
    EXPECT_EQ(seeded->repetitions(), 5);
    EXPECT_DOUBLE_EQ(seeded->weight(), 82.5);
}

TEST_F(SetSeedingTest, TheSeedCarriesNoIdentityAndIsNotCompleted)
{
    Set performed = completedSet(5, 82.5);
    performed.setId(77);
    performed.setExerciseId(12);

    const std::optional<Set> seeded
        = seedFromPreviousExercise(benchWith({ performed }), plannedSet(8, 0.0));

    ASSERT_TRUE(seeded.has_value());
    EXPECT_EQ(seeded->id(), -1);
    EXPECT_EQ(seeded->exerciseId(), -1);
    EXPECT_FALSE(seeded->completed());
}

TEST_F(SetSeedingTest, TheSeedKeepsThePositionOfTheSetItReplaces)
{
    Set fallback = plannedSet(8, 0.0);
    fallback.setPosition(3);

    const std::optional<Set> seeded
        = seedFromPreviousExercise(benchWith({ completedSet(5, 82.5) }), fallback);

    ASSERT_TRUE(seeded.has_value());
    EXPECT_EQ(seeded->position(), 3);
}

TEST_F(SetSeedingTest, APreviousSetOfADifferentMetricIsNotUsed)
{
    Exercise plank(QStringLiteral("Plank"), 60);
    Set held = Set::createDuration(90);
    held.setCompleted(true);
    plank.addSet(held);

    EXPECT_FALSE(seedFromPreviousExercise(plank, plannedSet(8, 0.0)).has_value());
}

TEST_F(SetSeedingTest, ATimedExerciseSeedsItsDuration)
{
    Exercise plank(QStringLiteral("Plank"), 60);
    Set held = Set::createDuration(90);
    held.setCompleted(true);
    plank.addSet(held);

    const std::optional<Set> seeded = seedFromPreviousExercise(plank, Set::createDuration(30));

    ASSERT_TRUE(seeded.has_value());
    EXPECT_EQ(seeded->durationSeconds(), 90);
}

TEST_F(SetSeedingTest, APreviousSetEqualToTheSeedChangesNothing)
{
    const std::optional<Set> seeded
        = seedFromPreviousExercise(benchWith({ completedSet(8, 0.0) }), plannedSet(8, 0.0));

    EXPECT_FALSE(seeded.has_value());
}

TEST_F(SetSeedingTest, SameSetValuesIgnoresIdentityAndCompletion)
{
    Set left(8, 60.0);
    Set right(8, 60.0);
    right.setId(9);
    right.setExerciseId(4);
    right.setCompleted(true);

    EXPECT_TRUE(sameSetValues(left, right));

    right.setWeight(62.5);
    EXPECT_FALSE(sameSetValues(left, right));
}
