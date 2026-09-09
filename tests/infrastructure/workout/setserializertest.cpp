#include "infrastructure/workout/setserializer.h"
#include "domain/workout/set.h"
#include <gtest/gtest.h>

class SetSerializerTest : public ::testing::Test
{
};

TEST_F(SetSerializerTest, ToVariant_FullSet)
{
    Set s;
    s.setId(5);
    s.setExerciseId(3);
    s.setRepetitions(12);
    s.setWeight(80.0);

    QVariantMap data = SetSerializer::toVariant(s);

    EXPECT_EQ(data.value(SetSerializer::id_key).toInt(), 5);
    EXPECT_EQ(data.value(SetSerializer::exercise_id_key).toInt(), 3);
    EXPECT_EQ(data.value(SetSerializer::repetitions_key).toInt(), 12);
    EXPECT_DOUBLE_EQ(data.value(SetSerializer::weight_key).toDouble(), 80.0);
}

TEST_F(SetSerializerTest, ToVariant_FractionalWeight)
{
    Set s(10, 25.5);

    QVariantMap data = SetSerializer::toVariant(s);

    EXPECT_DOUBLE_EQ(data.value(SetSerializer::weight_key).toDouble(), 25.5);
}

TEST_F(SetSerializerTest, ToVariant_NewSet_NoIds)
{
    Set s(10, 60.0);

    QVariantMap data = SetSerializer::toVariant(s);

    EXPECT_FALSE(data.contains(SetSerializer::id_key));
    EXPECT_FALSE(data.contains(SetSerializer::exercise_id_key));
    EXPECT_EQ(data.value(SetSerializer::repetitions_key).toInt(), 10);
    EXPECT_DOUBLE_EQ(data.value(SetSerializer::weight_key).toDouble(), 60.0);
}

TEST_F(SetSerializerTest, FromVariant_AllFields)
{
    QVariantMap data;
    data.insert(SetSerializer::id_key, 8);
    data.insert(SetSerializer::exercise_id_key, 4);
    data.insert(SetSerializer::repetitions_key, 15);
    data.insert(SetSerializer::weight_key, 50.0);

    Set s = SetSerializer::fromVariant(data);

    EXPECT_EQ(s.id(), 8);
    EXPECT_EQ(s.exerciseId(), 4);
    EXPECT_EQ(s.repetitions(), 15);
    EXPECT_DOUBLE_EQ(s.weight(), 50.0);
}

TEST_F(SetSerializerTest, FromVariant_FractionalWeight)
{
    QVariantMap data;
    data.insert(SetSerializer::repetitions_key, 8);
    data.insert(SetSerializer::weight_key, 25.5);

    Set s = SetSerializer::fromVariant(data);

    EXPECT_DOUBLE_EQ(s.weight(), 25.5);
}

TEST_F(SetSerializerTest, FromVariant_PartialFields)
{
    QVariantMap data;
    data.insert(SetSerializer::repetitions_key, 5);
    data.insert(SetSerializer::weight_key, 100.0);

    Set s = SetSerializer::fromVariant(data);

    EXPECT_EQ(s.id(), -1);
    EXPECT_EQ(s.exerciseId(), -1);
    EXPECT_EQ(s.repetitions(), 5);
    EXPECT_DOUBLE_EQ(s.weight(), 100.0);
}

TEST_F(SetSerializerTest, Roundtrip_PreservesData)
{
    Set original;
    original.setId(2);
    original.setExerciseId(1);
    original.setRepetitions(8);
    original.setWeight(120.0);

    QVariantMap data = SetSerializer::toVariant(original);
    Set restored = SetSerializer::fromVariant(data);

    EXPECT_EQ(restored.id(), original.id());
    EXPECT_EQ(restored.exerciseId(), original.exerciseId());
    EXPECT_EQ(restored.repetitions(), original.repetitions());
    EXPECT_DOUBLE_EQ(restored.weight(), original.weight());
}

TEST_F(SetSerializerTest, Roundtrip_FractionalWeight)
{
    Set original(6, 27.5);
    original.setId(3);
    original.setExerciseId(1);

    QVariantMap data = SetSerializer::toVariant(original);
    Set restored = SetSerializer::fromVariant(data);

    EXPECT_EQ(restored.repetitions(), 6);
    EXPECT_DOUBLE_EQ(restored.weight(), 27.5);
    EXPECT_EQ(restored.id(), 3);
}

TEST_F(SetSerializerTest, TableName_IsCorrect) { EXPECT_STREQ(SetSerializer::table, "sets"); }

TEST_F(SetSerializerTest, ToVariant_IncludesMetricAndLoadType)
{
    Set s(10, 80.0);

    QVariantMap data = SetSerializer::toVariant(s);

    EXPECT_EQ(data.value(SetSerializer::metric_key).toString(), "reps");
    EXPECT_EQ(data.value(SetSerializer::load_type_key).toString(), "external");
    EXPECT_EQ(data.value(SetSerializer::duration_seconds_key).toInt(), 0);
    EXPECT_DOUBLE_EQ(data.value(SetSerializer::distance_meters_key).toDouble(), 0.0);
    EXPECT_EQ(data.value(SetSerializer::rest_seconds_override_key).toInt(), -1);
}

TEST_F(SetSerializerTest, Roundtrip_TimedSetWithRestOverride)
{
    Set original = Set::createDuration(20);
    original.setRestSecondsOverride(10);
    original.setCompleted(true);

    Set restored = SetSerializer::fromVariant(SetSerializer::toVariant(original));

    EXPECT_EQ(restored.metric(), SetMetric::Duration);
    EXPECT_EQ(restored.loadType(), LoadType::None);
    EXPECT_EQ(restored.durationSeconds(), 20);
    EXPECT_EQ(restored.restSecondsOverride(), 10);
    EXPECT_TRUE(restored.completed());
}

TEST_F(SetSerializerTest, Roundtrip_DistanceSet)
{
    Set original = Set::createDistance(5000.0, 1500);

    Set restored = SetSerializer::fromVariant(SetSerializer::toVariant(original));

    EXPECT_EQ(restored.metric(), SetMetric::Distance);
    EXPECT_DOUBLE_EQ(restored.distanceMeters(), 5000.0);
    EXPECT_EQ(restored.durationSeconds(), 1500);
}

TEST_F(SetSerializerTest, Roundtrip_BodyweightSet)
{
    Set original(12, 0.0);
    original.setLoadType(LoadType::Bodyweight);

    Set restored = SetSerializer::fromVariant(SetSerializer::toVariant(original));

    EXPECT_EQ(restored.metric(), SetMetric::Reps);
    EXPECT_EQ(restored.loadType(), LoadType::Bodyweight);
    EXPECT_EQ(restored.repetitions(), 12);
    EXPECT_FALSE(restored.isWeighted());
}

TEST_F(SetSerializerTest, FromVariant_LegacyRow_FallsBackToWeightedReps)
{
    QVariantMap legacy;
    legacy.insert(SetSerializer::id_key, 3);
    legacy.insert(SetSerializer::exercise_id_key, 1);
    legacy.insert(SetSerializer::repetitions_key, 10);
    legacy.insert(SetSerializer::weight_key, 80.0);
    legacy.insert(SetSerializer::completed_key, 1);

    Set s = SetSerializer::fromVariant(legacy);

    EXPECT_EQ(s.metric(), SetMetric::Reps);
    EXPECT_EQ(s.loadType(), LoadType::External);
    EXPECT_EQ(s.durationSeconds(), 0);
    EXPECT_DOUBLE_EQ(s.distanceMeters(), 0.0);
    EXPECT_EQ(s.restSecondsOverride(), -1);
    EXPECT_TRUE(s.isWeighted());
}
