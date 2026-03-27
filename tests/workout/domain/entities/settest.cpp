#include <gtest/gtest.h>
#include "modules/workout/domain/entities/set.h"

class SetTest : public ::testing::Test
{
};

TEST_F(SetTest, DefaultConstructor_SetsDefaults)
{
    Set s;

    EXPECT_EQ(s.id(), -1);
    EXPECT_EQ(s.exerciseId(), -1);
    EXPECT_EQ(s.repetitions(), 0);
    EXPECT_EQ(s.weight(), 0);
    EXPECT_FALSE(s.completed());
}

TEST_F(SetTest, ParameterizedConstructor_SetsRepsAndWeight)
{
    Set s(12, 80);

    EXPECT_EQ(s.repetitions(), 12);
    EXPECT_EQ(s.weight(), 80);
    EXPECT_EQ(s.id(), -1);
    EXPECT_EQ(s.exerciseId(), -1);
    EXPECT_FALSE(s.completed());
}

TEST_F(SetTest, Setters_UpdateAllFields)
{
    Set s;

    s.setId(7);
    s.setExerciseId(3);
    s.setRepetitions(10);
    s.setWeight(100);
    s.setCompleted(true);

    EXPECT_EQ(s.id(), 7);
    EXPECT_EQ(s.exerciseId(), 3);
    EXPECT_EQ(s.repetitions(), 10);
    EXPECT_EQ(s.weight(), 100);
    EXPECT_TRUE(s.completed());
}

TEST_F(SetTest, CompletedToggle)
{
    Set s(8, 60);
    EXPECT_FALSE(s.completed());

    s.setCompleted(true);
    EXPECT_TRUE(s.completed());

    s.setCompleted(false);
    EXPECT_FALSE(s.completed());
}

TEST_F(SetTest, CopySemantics)
{
    Set original(15, 50);
    original.setId(1);
    original.setExerciseId(2);
    original.setCompleted(true);

    Set copy = original;

    EXPECT_EQ(copy.id(), original.id());
    EXPECT_EQ(copy.exerciseId(), original.exerciseId());
    EXPECT_EQ(copy.repetitions(), original.repetitions());
    EXPECT_EQ(copy.weight(), original.weight());
    EXPECT_EQ(copy.completed(), original.completed());
}
