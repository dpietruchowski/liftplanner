#include "domain/workout/exerciseseeding.h"

#include <gtest/gtest.h>

TEST(ExerciseSeedingTest, ARepsSetStartsAtEightRepetitions)
{
    const Set set = seedSet(SetMetric::Reps, LoadType::External);

    EXPECT_EQ(set.metric(), SetMetric::Reps);
    EXPECT_EQ(set.loadType(), LoadType::External);
    EXPECT_EQ(set.repetitions(), 8);
    EXPECT_FALSE(set.completed());
}

TEST(ExerciseSeedingTest, ADurationSetStartsAtThirtySeconds)
{
    const Set set = seedSet(SetMetric::Duration, LoadType::None);

    EXPECT_EQ(set.durationSeconds(), 30);
    EXPECT_EQ(set.repetitions(), 0);
}

TEST(ExerciseSeedingTest, ADistanceSetStartsAtOneKilometre)
{
    const Set set = seedSet(SetMetric::Distance, LoadType::None);

    EXPECT_DOUBLE_EQ(set.distanceMeters(), 1000.0);
}

TEST(ExerciseSeedingTest, AKindWithoutAPreferenceUsesItsOwnDefaults)
{
    const Set set = seedSetForKind(ExerciseKind::Cardio);

    EXPECT_EQ(set.metric(), SetMetric::Distance);
    EXPECT_EQ(set.loadType(), LoadType::None);
}

TEST(ExerciseSeedingTest, APreferredMetricThatSuitsTheKindIsKept)
{
    const Set set = seedSetForDefaults(ExerciseKind::Cardio, SetMetric::Duration, LoadType::None);

    EXPECT_EQ(set.metric(), SetMetric::Duration);
    EXPECT_EQ(set.durationSeconds(), 30);
}

TEST(ExerciseSeedingTest, APreferredMetricThatClashesWithTheKindIsDropped)
{
    const Set set
        = seedSetForDefaults(ExerciseKind::Strength, SetMetric::Duration, LoadType::External);

    EXPECT_EQ(set.metric(), SetMetric::Reps);
    EXPECT_EQ(set.repetitions(), 8);
}

TEST(ExerciseSeedingTest, ASeededExerciseCarriesTheDefinitionAndOneOpenSet)
{
    const Exercise exercise
        = seededExercise(7, QStringLiteral("Back Squat"), ExerciseKind::Strength, 180,
                         SetMetric::Reps, LoadType::External);

    EXPECT_EQ(exercise.name(), QStringLiteral("Back Squat"));
    EXPECT_EQ(exercise.kind(), ExerciseKind::Strength);
    EXPECT_EQ(exercise.restSeconds(), 180);
    ASSERT_TRUE(exercise.hasDefinition());
    EXPECT_EQ(exercise.definitionId().value(), 7);

    ASSERT_EQ(exercise.sets().size(), 1u);
    EXPECT_EQ(exercise.sets().front().repetitions(), 8);
    EXPECT_FALSE(exercise.sets().front().completed());
}
