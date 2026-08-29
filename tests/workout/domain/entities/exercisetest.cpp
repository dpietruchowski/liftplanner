#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
#include <gtest/gtest.h>

class ExerciseTest : public ::testing::Test
{
};

TEST_F(ExerciseTest, DefaultConstructor_SetsDefaults)
{
    Exercise e;

    EXPECT_EQ(e.id(), -1);
    EXPECT_EQ(e.workoutId(), -1);
    EXPECT_TRUE(e.name().isEmpty());
    EXPECT_TRUE(e.description().isEmpty());
    EXPECT_EQ(e.restSeconds(), 120);
}

TEST_F(ExerciseTest, ParameterizedConstructor_SetsNameAndRest)
{
    Exercise e("Bench Press", 90);

    EXPECT_EQ(e.name(), "Bench Press");
    EXPECT_EQ(e.restSeconds(), 90);
    EXPECT_EQ(e.id(), -1);
    EXPECT_EQ(e.workoutId(), -1);
}

TEST_F(ExerciseTest, Setters_UpdateAllFields)
{
    Exercise e;

    e.setId(10);
    e.setWorkoutId(5);
    e.setName("Squat");
    e.setDescription("Barbell back squat");
    e.setRestSeconds(180);

    EXPECT_EQ(e.id(), 10);
    EXPECT_EQ(e.workoutId(), 5);
    EXPECT_EQ(e.name(), "Squat");
    EXPECT_EQ(e.description(), "Barbell back squat");
    EXPECT_EQ(e.restSeconds(), 180);
}

TEST_F(ExerciseTest, DefaultRestSeconds_Is120)
{
    Exercise e("Deadlift", 120);
    EXPECT_EQ(e.restSeconds(), 120);

    Exercise e2;
    EXPECT_EQ(e2.restSeconds(), 120);
}

TEST_F(ExerciseTest, CopySemantics)
{
    Exercise original("Pullup", 60);
    original.setId(3);
    original.setWorkoutId(1);
    original.setDescription("Bodyweight pullup");

    Exercise copy = original;

    EXPECT_EQ(copy.id(), original.id());
    EXPECT_EQ(copy.name(), original.name());
    EXPECT_EQ(copy.restSeconds(), original.restSeconds());
    EXPECT_EQ(copy.description(), original.description());
}

TEST_F(ExerciseTest, AddSet_AppendsSets)
{
    Exercise e("Bench Press", 90);
    EXPECT_TRUE(e.sets().empty());

    Set s1(10, 80);
    Set s2(8, 85);
    e.addSet(s1);
    e.addSet(s2);

    EXPECT_EQ(e.sets().size(), 2u);
    EXPECT_EQ(e.sets()[0].repetitions(), 10);
    EXPECT_EQ(e.sets()[0].weight(), 80);
    EXPECT_EQ(e.sets()[1].repetitions(), 8);
    EXPECT_EQ(e.sets()[1].weight(), 85);
}

TEST_F(ExerciseTest, RemoveSet_RemovesByIndex)
{
    Exercise e("Squat", 120);
    e.addSet(Set(10, 100));
    e.addSet(Set(8, 110));
    e.addSet(Set(6, 120));

    e.removeSet(1);

    EXPECT_EQ(e.sets().size(), 2u);
    EXPECT_EQ(e.sets()[0].weight(), 100);
    EXPECT_EQ(e.sets()[1].weight(), 120);
}

TEST_F(ExerciseTest, RemoveSet_InvalidIndex_DoesNothing)
{
    Exercise e("Deadlift", 120);
    e.addSet(Set(5, 140));

    e.removeSet(-1);
    e.removeSet(5);

    EXPECT_EQ(e.sets().size(), 1u);
}

TEST_F(ExerciseTest, IsCompleted_EmptySets_ReturnsFalse)
{
    Exercise e("Pullup", 60);
    EXPECT_FALSE(e.isCompleted());
}

TEST_F(ExerciseTest, IsCompleted_AllCompleted_ReturnsTrue)
{
    Exercise e("Bench Press", 90);
    Set s1(10, 80);
    s1.setCompleted(true);
    Set s2(8, 85);
    s2.setCompleted(true);
    e.addSet(s1);
    e.addSet(s2);

    EXPECT_TRUE(e.isCompleted());
}

TEST_F(ExerciseTest, IsCompleted_SomeNotCompleted_ReturnsFalse)
{
    Exercise e("Bench Press", 90);
    Set s1(10, 80);
    s1.setCompleted(true);
    Set s2(8, 85);
    e.addSet(s1);
    e.addSet(s2);

    EXPECT_FALSE(e.isCompleted());
}

TEST_F(ExerciseTest, SetsToString_FormatsCorrectly)
{
    Exercise e("Bench Press", 90);
    e.addSet(Set(10, 80));
    e.addSet(Set(8, 85));
    e.addSet(Set(6, 90));

    EXPECT_EQ(e.setsToString(), "10x80kg, 8x85kg, 6x90kg");
}

TEST_F(ExerciseTest, SetsToString_Empty_ReturnsEmpty)
{
    Exercise e("Empty", 60);
    EXPECT_EQ(e.setsToString(), "");
}

TEST_F(ExerciseTest, Aggregates_SumAcrossSets)
{
    Exercise e("Bench Press", 90);
    e.addSet(Set(5, 100));  // tw 500, 1RM 112.5
    e.addSet(Set(5, 80));  // tw 400, 1RM 90
    e.addSet(Set(10, 60));  // tw 600, 1RM 80

    EXPECT_DOUBLE_EQ(e.totalWeight(), 1500.0);
    EXPECT_EQ(e.totalRepetitions(), 20);
    EXPECT_DOUBLE_EQ(e.averageWeight(), 80.0);  // (100 + 80 + 60) / 3
    EXPECT_DOUBLE_EQ(e.bestOneRepMax(), 112.5);
}

TEST_F(ExerciseTest, Aggregates_NoSets_AreZero)
{
    Exercise e("Empty", 60);

    EXPECT_DOUBLE_EQ(e.totalWeight(), 0.0);
    EXPECT_EQ(e.totalRepetitions(), 0);
    EXPECT_DOUBLE_EQ(e.averageWeight(), 0.0);
    EXPECT_DOUBLE_EQ(e.bestOneRepMax(), 0.0);
}

TEST_F(ExerciseTest, CopySemantics_IncludesSets)
{
    Exercise original("Bench", 90);
    original.addSet(Set(10, 80));
    original.addSet(Set(8, 85));

    Exercise copy = original;

    EXPECT_EQ(copy.sets().size(), 2u);
    EXPECT_EQ(copy.sets()[0].repetitions(), 10);
    EXPECT_EQ(copy.sets()[1].weight(), 85);
}

TEST_F(ExerciseTest, DefaultKind_IsStrength)
{
    Exercise e;
    EXPECT_EQ(e.kind(), ExerciseKind::Strength);

    e.setKind(ExerciseKind::Interval);
    EXPECT_EQ(e.kind(), ExerciseKind::Interval);
}

TEST_F(ExerciseTest, KindToString_Roundtrips)
{
    EXPECT_EQ(exerciseKindFromString(exerciseKindToString(ExerciseKind::Strength)),
              ExerciseKind::Strength);
    EXPECT_EQ(exerciseKindFromString(exerciseKindToString(ExerciseKind::Bodyweight)),
              ExerciseKind::Bodyweight);
    EXPECT_EQ(exerciseKindFromString(exerciseKindToString(ExerciseKind::Cardio)),
              ExerciseKind::Cardio);
    EXPECT_EQ(exerciseKindFromString(exerciseKindToString(ExerciseKind::Interval)),
              ExerciseKind::Interval);
    EXPECT_EQ(exerciseKindFromString(exerciseKindToString(ExerciseKind::Mobility)),
              ExerciseKind::Mobility);
    EXPECT_EQ(exerciseKindFromString("nonsense"), ExerciseKind::Strength);
}

TEST_F(ExerciseTest, RestSecondsForSet_NoOverride_UsesExerciseDefault)
{
    Exercise e("Bench Press", 90);
    e.addSet(Set(10, 80));
    e.addSet(Set(8, 85));

    EXPECT_EQ(e.restSecondsForSet(0), 90);
    EXPECT_EQ(e.restSecondsForSet(1), 90);
}

TEST_F(ExerciseTest, RestSecondsForSet_Override_WinsOverDefault)
{
    Exercise e("Tabata", 60);
    Set work = Set::createDuration(20);
    work.setRestSecondsOverride(10);
    e.addSet(work);
    e.addSet(Set::createDuration(20));

    EXPECT_EQ(e.restSecondsForSet(0), 10);
    EXPECT_EQ(e.restSecondsForSet(1), 60);
}

TEST_F(ExerciseTest, RestSecondsForSet_ZeroOverride_MeansNoRest)
{
    Exercise e("Circuit", 60);
    Set work = Set::createDuration(30);
    work.setRestSecondsOverride(0);
    e.addSet(work);

    EXPECT_EQ(e.restSecondsForSet(0), 0);
}

TEST_F(ExerciseTest, RestSecondsForSet_InvalidIndex_UsesExerciseDefault)
{
    Exercise e("Squat", 180);
    e.addSet(Set(5, 100));

    EXPECT_EQ(e.restSecondsForSet(-1), 180);
    EXPECT_EQ(e.restSecondsForSet(7), 180);
}

TEST_F(ExerciseTest, TotalDurationAndDistance_SumAcrossSets)
{
    Exercise e("Intervals", 60);
    e.addSet(Set::createDuration(20));
    e.addSet(Set::createDuration(20));
    e.addSet(Set::createDistance(400.0, 90));

    EXPECT_EQ(e.totalDurationSeconds(), 130);
    EXPECT_DOUBLE_EQ(e.totalDistanceMeters(), 400.0);
}

TEST_F(ExerciseTest, TotalDurationAndDistance_StrengthSets_AreZero)
{
    Exercise e("Bench Press", 90);
    e.addSet(Set(10, 80));

    EXPECT_EQ(e.totalDurationSeconds(), 0);
    EXPECT_DOUBLE_EQ(e.totalDistanceMeters(), 0.0);
}

TEST_F(ExerciseTest, Aggregates_BodyweightSets_ExcludedFromWeightMetrics)
{
    Exercise e("Pullup", 90);
    Set s1(12, 0);
    s1.setLoadType(LoadType::Bodyweight);
    Set s2(10, 0);
    s2.setLoadType(LoadType::Bodyweight);
    e.addSet(s1);
    e.addSet(s2);

    EXPECT_DOUBLE_EQ(e.totalWeight(), 0.0);
    EXPECT_DOUBLE_EQ(e.averageWeight(), 0.0);
    EXPECT_DOUBLE_EQ(e.bestOneRepMax(), 0.0);
    EXPECT_EQ(e.totalRepetitions(), 22);
}

TEST_F(ExerciseTest, Aggregates_MixedExercise_CountsOnlyWeightedSets)
{
    Exercise e("Mixed", 90);
    e.addSet(Set(5, 100));
    e.addSet(Set(5, 80));
    e.addSet(Set::createDuration(60));
    Set assisted(8, 20);
    assisted.setLoadType(LoadType::Assisted);
    e.addSet(assisted);

    EXPECT_DOUBLE_EQ(e.totalWeight(), 900.0);
    EXPECT_DOUBLE_EQ(e.averageWeight(), 90.0);
    EXPECT_DOUBLE_EQ(e.bestOneRepMax(), 112.5);
    EXPECT_EQ(e.totalRepetitions(), 18);
    EXPECT_EQ(e.totalDurationSeconds(), 60);
}

TEST_F(ExerciseTest, IsCompleted_TimedSets_BehaveLikeRepSets)
{
    Exercise e("Plank", 60);
    Set s1 = Set::createDuration(45);
    Set s2 = Set::createDuration(45);
    e.addSet(s1);
    e.addSet(s2);

    EXPECT_FALSE(e.isCompleted());

    e.sets()[0].setCompleted(true);
    EXPECT_FALSE(e.isCompleted());

    e.sets()[1].setCompleted(true);
    EXPECT_TRUE(e.isCompleted());
}

TEST_F(ExerciseTest, IsWeighted_TrueWhenAnySetCarriesExternalLoad)
{
    Exercise e("Bench Press", 120);
    e.addSet(Set(10, 0.0));
    e.addSet(Set(8, 80.0));

    EXPECT_TRUE(e.isWeighted());
}

TEST_F(ExerciseTest, IsWeighted_FalseForTimedDistanceAndBodyweightWork)
{
    Exercise plank("Plank", 60);
    plank.addSet(Set::createDuration(45));
    EXPECT_FALSE(plank.isWeighted());

    Exercise run("Run", 0);
    run.addSet(Set::createDistance(5000.0, 1440));
    EXPECT_FALSE(run.isWeighted());

    Exercise pullups("Pull-ups", 90);
    Set bodyweight(8, 0.0);
    bodyweight.setLoadType(LoadType::Bodyweight);
    pullups.addSet(bodyweight);
    EXPECT_FALSE(pullups.isWeighted());

    EXPECT_FALSE(Exercise("Empty", 60).isWeighted());
}
