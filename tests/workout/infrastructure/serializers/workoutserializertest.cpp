#include <gtest/gtest.h>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/infrastructure/serializers/workoutserializer.h"

class WorkoutSerializerTest : public ::testing::Test
{
};

TEST_F(WorkoutSerializerTest, ToVariant_FullWorkout)
{
    Workout w;
    w.setId(42);
    w.setName("Push Day");
    w.setCreatedTime(QDateTime(QDate(2025, 3, 15), QTime(10, 0)));
    w.setStartedTime(QDateTime(QDate(2025, 3, 15), QTime(10, 30)));
    w.setEndedTime(QDateTime(QDate(2025, 3, 15), QTime(11, 30)));

    QVariantMap data = WorkoutSerializer::toVariant(w);

    EXPECT_EQ(data.value(WorkoutSerializer::id_key).toInt(), 42);
    EXPECT_EQ(data.value(WorkoutSerializer::name_key).toString(), "Push Day");
    EXPECT_TRUE(data.contains(WorkoutSerializer::created_time_key));
    EXPECT_TRUE(data.contains(WorkoutSerializer::started_time_key));
    EXPECT_TRUE(data.contains(WorkoutSerializer::ended_time_key));
}

TEST_F(WorkoutSerializerTest, ToVariant_NewWorkout_NoId)
{
    Workout w("Test", QDateTime::currentDateTime());

    QVariantMap data = WorkoutSerializer::toVariant(w);

    EXPECT_FALSE(data.contains(WorkoutSerializer::id_key));
    EXPECT_EQ(data.value(WorkoutSerializer::name_key).toString(), "Test");
}

TEST_F(WorkoutSerializerTest, FromVariant_AllFields)
{
    QVariantMap data;
    data.insert(WorkoutSerializer::id_key, 10);
    data.insert(WorkoutSerializer::name_key, "Leg Day");
    data.insert(WorkoutSerializer::created_time_key, "2025-03-15T10:00:00");
    data.insert(WorkoutSerializer::started_time_key, "2025-03-15T10:30:00");
    data.insert(WorkoutSerializer::ended_time_key, "2025-03-15T11:30:00");

    Workout w = WorkoutSerializer::fromVariant(data);

    EXPECT_EQ(w.id(), 10);
    EXPECT_EQ(w.name(), "Leg Day");
    EXPECT_TRUE(w.createdTime().isValid());
    EXPECT_TRUE(w.startedTime().isValid());
    EXPECT_TRUE(w.endedTime().isValid());
}

TEST_F(WorkoutSerializerTest, FromVariant_PartialFields)
{
    QVariantMap data;
    data.insert(WorkoutSerializer::name_key, "Minimal");

    Workout w = WorkoutSerializer::fromVariant(data);

    EXPECT_EQ(w.id(), -1);
    EXPECT_EQ(w.name(), "Minimal");
    EXPECT_FALSE(w.startedTime().isValid());
    EXPECT_FALSE(w.endedTime().isValid());
}

TEST_F(WorkoutSerializerTest, Roundtrip_PreservesData)
{
    Workout original;
    original.setId(5);
    original.setName("Full Body");
    original.setCreatedTime(QDateTime(QDate(2025, 6, 1), QTime(8, 0)));
    original.setStartedTime(QDateTime(QDate(2025, 6, 1), QTime(8, 15)));

    QVariantMap data = WorkoutSerializer::toVariant(original);
    Workout restored = WorkoutSerializer::fromVariant(data);

    EXPECT_EQ(restored.id(), original.id());
    EXPECT_EQ(restored.name(), original.name());
    EXPECT_EQ(restored.createdTime(), original.createdTime());
    EXPECT_EQ(restored.startedTime(), original.startedTime());
}

TEST_F(WorkoutSerializerTest, TableName_IsCorrect)
{
    EXPECT_STREQ(WorkoutSerializer::table, "workouts");
}
