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
    EXPECT_TRUE(e.youtubeLink().isEmpty());
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
    e.setYoutubeLink("https://youtube.com/watch?v=abc");

    EXPECT_EQ(e.id(), 10);
    EXPECT_EQ(e.workoutId(), 5);
    EXPECT_EQ(e.name(), "Squat");
    EXPECT_EQ(e.description(), "Barbell back squat");
    EXPECT_EQ(e.restSeconds(), 180);
    EXPECT_EQ(e.youtubeLink(), "https://youtube.com/watch?v=abc");
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
    e.addSet(Set(5, 80));   // tw 400, 1RM 90
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
