#include <gtest/gtest.h>
#include "modules/workout/domain/entities/workout.h"

class WorkoutTest : public ::testing::Test
{
};

TEST_F(WorkoutTest, DefaultConstructor_SetsDefaults)
{
    Workout w;

    EXPECT_EQ(w.id(), -1);
    EXPECT_TRUE(w.name().isEmpty());
    EXPECT_FALSE(w.createdTime().isValid());
    EXPECT_FALSE(w.startedTime().isValid());
    EXPECT_FALSE(w.endedTime().isValid());
}

TEST_F(WorkoutTest, ParameterizedConstructor_SetsNameAndCreatedTime)
{
    QDateTime now = QDateTime::currentDateTime();
    Workout w("Push Day", now);

    EXPECT_EQ(w.name(), "Push Day");
    EXPECT_EQ(w.createdTime(), now);
    EXPECT_EQ(w.id(), -1);
}

TEST_F(WorkoutTest, Setters_UpdateAllFields)
{
    Workout w;
    QDateTime created(QDate(2025, 1, 15), QTime(10, 0));
    QDateTime started(QDate(2025, 1, 15), QTime(10, 30));
    QDateTime ended(QDate(2025, 1, 15), QTime(11, 30));

    w.setId(42);
    w.setName("Leg Day");
    w.setCreatedTime(created);
    w.setStartedTime(started);
    w.setEndedTime(ended);

    EXPECT_EQ(w.id(), 42);
    EXPECT_EQ(w.name(), "Leg Day");
    EXPECT_EQ(w.createdTime(), created);
    EXPECT_EQ(w.startedTime(), started);
    EXPECT_EQ(w.endedTime(), ended);
}

TEST_F(WorkoutTest, CreateDefault_SetsNameAndCurrentTime)
{
    QDateTime before = QDateTime::currentDateTime();
    Workout w = Workout::createDefault("Pull Day");
    QDateTime after = QDateTime::currentDateTime();

    EXPECT_EQ(w.name(), "Pull Day");
    EXPECT_EQ(w.id(), -1);
    EXPECT_GE(w.createdTime(), before);
    EXPECT_LE(w.createdTime(), after);
}

TEST_F(WorkoutTest, CopySemantics)
{
    Workout original("Test", QDateTime::currentDateTime());
    original.setId(5);

    Workout copy = original;

    EXPECT_EQ(copy.id(), original.id());
    EXPECT_EQ(copy.name(), original.name());
    EXPECT_EQ(copy.createdTime(), original.createdTime());
}
