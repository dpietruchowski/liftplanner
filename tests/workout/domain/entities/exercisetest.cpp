#include <gtest/gtest.h>
#include "modules/workout/domain/entities/exercise.h"

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
