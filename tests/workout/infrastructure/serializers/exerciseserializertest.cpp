#include <gtest/gtest.h>
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/infrastructure/serializers/exerciseserializer.h"

class ExerciseSerializerTest : public ::testing::Test
{
};

TEST_F(ExerciseSerializerTest, ToVariant_FullExercise)
{
    Exercise e;
    e.setId(10);
    e.setWorkoutId(5);
    e.setName("Bench Press");
    e.setDescription("Flat barbell bench press");
    e.setRestSeconds(120);
    e.setYoutubeLink("https://youtube.com/watch?v=abc");

    QVariantMap data = ExerciseSerializer::toVariant(e);

    EXPECT_EQ(data.value(ExerciseSerializer::id_key).toInt(), 10);
    EXPECT_EQ(data.value(ExerciseSerializer::workout_id_key).toInt(), 5);
    EXPECT_EQ(data.value(ExerciseSerializer::name_key).toString(), "Bench Press");
    EXPECT_EQ(data.value(ExerciseSerializer::description_key).toString(), "Flat barbell bench press");
    EXPECT_EQ(data.value(ExerciseSerializer::rest_seconds_key).toInt(), 120);
    EXPECT_EQ(data.value(ExerciseSerializer::youtube_link_key).toString(), "https://youtube.com/watch?v=abc");
}

TEST_F(ExerciseSerializerTest, ToVariant_NewExercise_NoId)
{
    Exercise e("Squat", 90);

    QVariantMap data = ExerciseSerializer::toVariant(e);

    EXPECT_FALSE(data.contains(ExerciseSerializer::id_key));
    EXPECT_FALSE(data.contains(ExerciseSerializer::workout_id_key));
    EXPECT_EQ(data.value(ExerciseSerializer::name_key).toString(), "Squat");
    EXPECT_EQ(data.value(ExerciseSerializer::rest_seconds_key).toInt(), 90);
}

TEST_F(ExerciseSerializerTest, FromVariant_AllFields)
{
    QVariantMap data;
    data.insert(ExerciseSerializer::id_key, 3);
    data.insert(ExerciseSerializer::workout_id_key, 1);
    data.insert(ExerciseSerializer::name_key, "Deadlift");
    data.insert(ExerciseSerializer::description_key, "Conventional deadlift");
    data.insert(ExerciseSerializer::rest_seconds_key, 180);
    data.insert(ExerciseSerializer::youtube_link_key, "https://youtube.com/xyz");

    Exercise e = ExerciseSerializer::fromVariant(data);

    EXPECT_EQ(e.id(), 3);
    EXPECT_EQ(e.workoutId(), 1);
    EXPECT_EQ(e.name(), "Deadlift");
    EXPECT_EQ(e.description(), "Conventional deadlift");
    EXPECT_EQ(e.restSeconds(), 180);
    EXPECT_EQ(e.youtubeLink(), "https://youtube.com/xyz");
}

TEST_F(ExerciseSerializerTest, Roundtrip_PreservesData)
{
    Exercise original;
    original.setId(7);
    original.setWorkoutId(2);
    original.setName("Pullup");
    original.setDescription("Bodyweight");
    original.setRestSeconds(60);
    original.setYoutubeLink("https://yt.com/pull");

    QVariantMap data = ExerciseSerializer::toVariant(original);
    Exercise restored = ExerciseSerializer::fromVariant(data);

    EXPECT_EQ(restored.id(), original.id());
    EXPECT_EQ(restored.workoutId(), original.workoutId());
    EXPECT_EQ(restored.name(), original.name());
    EXPECT_EQ(restored.description(), original.description());
    EXPECT_EQ(restored.restSeconds(), original.restSeconds());
    EXPECT_EQ(restored.youtubeLink(), original.youtubeLink());
}

TEST_F(ExerciseSerializerTest, TableName_IsCorrect)
{
    EXPECT_STREQ(ExerciseSerializer::table, "exercises");
}
