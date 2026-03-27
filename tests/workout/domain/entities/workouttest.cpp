#include <gtest/gtest.h>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"

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

TEST_F(WorkoutTest, AddExercise_AppendsExercises)
{
    Workout w("Push Day", QDateTime::currentDateTime());
    EXPECT_TRUE(w.exercises().empty());

    Exercise e1("Bench Press", 90);
    Exercise e2("Overhead Press", 120);
    w.addExercise(e1);
    w.addExercise(e2);

    EXPECT_EQ(w.exercises().size(), 2u);
    EXPECT_EQ(w.exercises()[0].name(), "Bench Press");
    EXPECT_EQ(w.exercises()[1].name(), "Overhead Press");
}

TEST_F(WorkoutTest, RemoveExercise_RemovesByIndex)
{
    Workout w("Push Day", QDateTime::currentDateTime());
    w.addExercise(Exercise("A", 60));
    w.addExercise(Exercise("B", 60));
    w.addExercise(Exercise("C", 60));

    w.removeExercise(1);

    EXPECT_EQ(w.exercises().size(), 2u);
    EXPECT_EQ(w.exercises()[0].name(), "A");
    EXPECT_EQ(w.exercises()[1].name(), "C");
}

TEST_F(WorkoutTest, RemoveExercise_InvalidIndex_DoesNothing)
{
    Workout w("Push Day", QDateTime::currentDateTime());
    w.addExercise(Exercise("A", 60));

    w.removeExercise(-1);
    w.removeExercise(5);

    EXPECT_EQ(w.exercises().size(), 1u);
}

TEST_F(WorkoutTest, IsCompleted_EmptyExercises_ReturnsFalse)
{
    Workout w;
    EXPECT_FALSE(w.isCompleted());
}

TEST_F(WorkoutTest, IsCompleted_AllExercisesCompleted_ReturnsTrue)
{
    Workout w("Push", QDateTime::currentDateTime());

    Exercise e1("Bench", 90);
    Set s1(10, 80);
    s1.setCompleted(true);
    e1.addSet(s1);

    Exercise e2("OHP", 120);
    Set s2(8, 40);
    s2.setCompleted(true);
    e2.addSet(s2);

    w.addExercise(e1);
    w.addExercise(e2);

    EXPECT_TRUE(w.isCompleted());
}

TEST_F(WorkoutTest, IsCompleted_SomeNotCompleted_ReturnsFalse)
{
    Workout w("Push", QDateTime::currentDateTime());

    Exercise e1("Bench", 90);
    Set s1(10, 80);
    s1.setCompleted(true);
    e1.addSet(s1);

    Exercise e2("OHP", 120);
    e2.addSet(Set(8, 40)); // not completed

    w.addExercise(e1);
    w.addExercise(e2);

    EXPECT_FALSE(w.isCompleted());
}

TEST_F(WorkoutTest, CopySemantics_IncludesExercises)
{
    Workout original("Push", QDateTime::currentDateTime());
    Exercise e("Bench", 90);
    e.addSet(Set(10, 80));
    original.addExercise(e);

    Workout copy = original;

    EXPECT_EQ(copy.exercises().size(), 1u);
    EXPECT_EQ(copy.exercises()[0].name(), "Bench");
    EXPECT_EQ(copy.exercises()[0].sets().size(), 1u);
    EXPECT_EQ(copy.exercises()[0].sets()[0].repetitions(), 10);
}
