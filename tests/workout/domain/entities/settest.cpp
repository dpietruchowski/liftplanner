#include "modules/workout/domain/entities/set.h"
#include <gtest/gtest.h>

class SetTest : public ::testing::Test
{
};

TEST_F(SetTest, DefaultConstructor_SetsDefaults)
{
    Set s;

    EXPECT_EQ(s.id(), -1);
    EXPECT_EQ(s.exerciseId(), -1);
    EXPECT_EQ(s.repetitions(), 0);
    EXPECT_DOUBLE_EQ(s.weight(), 0.0);
    EXPECT_FALSE(s.completed());
}

TEST_F(SetTest, ParameterizedConstructor_SetsRepsAndWeight)
{
    Set s(12, 80);

    EXPECT_EQ(s.repetitions(), 12);
    EXPECT_DOUBLE_EQ(s.weight(), 80.0);
    EXPECT_EQ(s.id(), -1);
    EXPECT_EQ(s.exerciseId(), -1);
    EXPECT_FALSE(s.completed());
}

TEST_F(SetTest, ParameterizedConstructor_FractionalWeight)
{
    Set s(10, 25.5);

    EXPECT_EQ(s.repetitions(), 10);
    EXPECT_DOUBLE_EQ(s.weight(), 25.5);
}

TEST_F(SetTest, Setters_UpdateAllFields)
{
    Set s;

    s.setId(7);
    s.setExerciseId(3);
    s.setRepetitions(10);
    s.setWeight(100.0);
    s.setCompleted(true);

    EXPECT_EQ(s.id(), 7);
    EXPECT_EQ(s.exerciseId(), 3);
    EXPECT_EQ(s.repetitions(), 10);
    EXPECT_DOUBLE_EQ(s.weight(), 100.0);
    EXPECT_TRUE(s.completed());
}

TEST_F(SetTest, SetWeight_AcceptsHalfKiloSteps)
{
    Set s;

    s.setWeight(25.5);
    EXPECT_DOUBLE_EQ(s.weight(), 25.5);

    s.setWeight(0.5);
    EXPECT_DOUBLE_EQ(s.weight(), 0.5);

    s.setWeight(100.5);
    EXPECT_DOUBLE_EQ(s.weight(), 100.5);
}

TEST_F(SetTest, SetWeight_IncrementByHalfKilo)
{
    Set s;
    s.setWeight(25.0);

    s.setWeight(s.weight() + 2.5);
    EXPECT_DOUBLE_EQ(s.weight(), 27.5);

    s.setWeight(s.weight() - 2.5);
    EXPECT_DOUBLE_EQ(s.weight(), 25.0);
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

TEST_F(SetTest, TotalWeight_RepsTimesWeight)
{
    EXPECT_DOUBLE_EQ(Set(5, 100).totalWeight(), 500.0);
    EXPECT_DOUBLE_EQ(Set(8, 60).totalWeight(), 480.0);
    EXPECT_DOUBLE_EQ(Set(0, 100).totalWeight(), 0.0);
}

TEST_F(SetTest, OneRepMax_ZeroReps_IsZero) { EXPECT_DOUBLE_EQ(Set(0, 100).oneRepMax(), 0.0); }

TEST_F(SetTest, OneRepMax_SingleRep_IsWeight) { EXPECT_DOUBLE_EQ(Set(1, 140).oneRepMax(), 140.0); }

TEST_F(SetTest, OneRepMax_TwoToTenReps_UsesBrzycki)
{
    // Brzycki: weight * 36 / (37 - reps)
    EXPECT_DOUBLE_EQ(Set(5, 100).oneRepMax(), 112.5);  // 3600 / 32
    EXPECT_DOUBLE_EQ(Set(10, 90).oneRepMax(), 120.0);  // 3240 / 27
}

TEST_F(SetTest, OneRepMax_AboveTenReps_UsesEpley)
{
    // Epley: weight * (1 + reps / 30)
    EXPECT_DOUBLE_EQ(Set(12, 60).oneRepMax(), 84.0);  // 60 * 1.4
    EXPECT_DOUBLE_EQ(Set(15, 100).oneRepMax(), 150.0);  // 100 * 1.5
}

TEST_F(SetTest, CopySemantics)
{
    Set original(15, 50.5);
    original.setId(1);
    original.setExerciseId(2);
    original.setCompleted(true);

    Set copy = original;

    EXPECT_EQ(copy.id(), original.id());
    EXPECT_EQ(copy.exerciseId(), original.exerciseId());
    EXPECT_EQ(copy.repetitions(), original.repetitions());
    EXPECT_DOUBLE_EQ(copy.weight(), original.weight());
    EXPECT_EQ(copy.completed(), original.completed());
}

TEST_F(SetTest, DefaultConstructor_IsWeightedReps)
{
    Set s;

    EXPECT_EQ(s.metric(), SetMetric::Reps);
    EXPECT_EQ(s.loadType(), LoadType::External);
    EXPECT_EQ(s.durationSeconds(), 0);
    EXPECT_DOUBLE_EQ(s.distanceMeters(), 0.0);
    EXPECT_EQ(s.restSecondsOverride(), -1);
    EXPECT_TRUE(s.isWeighted());
}

TEST_F(SetTest, CreateDuration_SetsMetricAndSeconds)
{
    Set s = Set::createDuration(45);

    EXPECT_EQ(s.metric(), SetMetric::Duration);
    EXPECT_EQ(s.loadType(), LoadType::None);
    EXPECT_EQ(s.durationSeconds(), 45);
    EXPECT_EQ(s.repetitions(), 0);
    EXPECT_FALSE(s.isWeighted());
}

TEST_F(SetTest, CreateDistance_SetsMetersAndSeconds)
{
    Set s = Set::createDistance(5000.0, 1440);

    EXPECT_EQ(s.metric(), SetMetric::Distance);
    EXPECT_EQ(s.loadType(), LoadType::None);
    EXPECT_DOUBLE_EQ(s.distanceMeters(), 5000.0);
    EXPECT_EQ(s.durationSeconds(), 1440);
    EXPECT_FALSE(s.isWeighted());
}

TEST_F(SetTest, IsWeighted_OnlyForExternalReps)
{
    Set external(10, 80);
    EXPECT_TRUE(external.isWeighted());

    Set bodyweight(12, 0);
    bodyweight.setLoadType(LoadType::Bodyweight);
    EXPECT_FALSE(bodyweight.isWeighted());

    Set added(8, 10);
    added.setLoadType(LoadType::Added);
    EXPECT_FALSE(added.isWeighted());

    Set assisted(8, 20);
    assisted.setLoadType(LoadType::Assisted);
    EXPECT_FALSE(assisted.isWeighted());

    Set band(15, 0);
    band.setLoadType(LoadType::Band);
    EXPECT_FALSE(band.isWeighted());
}

TEST_F(SetTest, TotalWeight_NonWeightedSets_AreZero)
{
    EXPECT_DOUBLE_EQ(Set::createDuration(60).totalWeight(), 0.0);
    EXPECT_DOUBLE_EQ(Set::createDistance(5000.0, 1440).totalWeight(), 0.0);

    Set bodyweight(12, 0);
    bodyweight.setLoadType(LoadType::Bodyweight);
    EXPECT_DOUBLE_EQ(bodyweight.totalWeight(), 0.0);

    Set added(8, 10);
    added.setLoadType(LoadType::Added);
    EXPECT_DOUBLE_EQ(added.totalWeight(), 0.0);
}

TEST_F(SetTest, OneRepMax_NonWeightedSets_AreZero)
{
    Set assisted(8, 20);
    assisted.setLoadType(LoadType::Assisted);
    EXPECT_DOUBLE_EQ(assisted.oneRepMax(), 0.0);

    Set bodyweight(5, 0);
    bodyweight.setLoadType(LoadType::Bodyweight);
    EXPECT_DOUBLE_EQ(bodyweight.oneRepMax(), 0.0);

    Set timed = Set::createDuration(30);
    timed.setWeight(100.0);
    EXPECT_DOUBLE_EQ(timed.oneRepMax(), 0.0);
}

TEST_F(SetTest, RestSecondsOverride_Roundtrips)
{
    Set s = Set::createDuration(20);
    EXPECT_EQ(s.restSecondsOverride(), -1);

    s.setRestSecondsOverride(10);
    EXPECT_EQ(s.restSecondsOverride(), 10);

    s.setRestSecondsOverride(0);
    EXPECT_EQ(s.restSecondsOverride(), 0);
}

TEST_F(SetTest, CopySemantics_IncludesNewFields)
{
    Set original = Set::createDistance(1000.0, 300);
    original.setRestSecondsOverride(90);
    original.setLoadType(LoadType::Band);

    Set copy = original;

    EXPECT_EQ(copy.metric(), original.metric());
    EXPECT_EQ(copy.loadType(), original.loadType());
    EXPECT_EQ(copy.durationSeconds(), original.durationSeconds());
    EXPECT_DOUBLE_EQ(copy.distanceMeters(), original.distanceMeters());
    EXPECT_EQ(copy.restSecondsOverride(), original.restSecondsOverride());
}

TEST_F(SetTest, MetricToString_Roundtrips)
{
    EXPECT_EQ(setMetricFromString(setMetricToString(SetMetric::Reps)), SetMetric::Reps);
    EXPECT_EQ(setMetricFromString(setMetricToString(SetMetric::Duration)), SetMetric::Duration);
    EXPECT_EQ(setMetricFromString(setMetricToString(SetMetric::Distance)), SetMetric::Distance);
    EXPECT_EQ(setMetricFromString("nonsense"), SetMetric::Reps);
}

TEST_F(SetTest, LoadTypeToString_Roundtrips)
{
    EXPECT_EQ(loadTypeFromString(loadTypeToString(LoadType::External)), LoadType::External);
    EXPECT_EQ(loadTypeFromString(loadTypeToString(LoadType::Bodyweight)), LoadType::Bodyweight);
    EXPECT_EQ(loadTypeFromString(loadTypeToString(LoadType::Added)), LoadType::Added);
    EXPECT_EQ(loadTypeFromString(loadTypeToString(LoadType::Assisted)), LoadType::Assisted);
    EXPECT_EQ(loadTypeFromString(loadTypeToString(LoadType::Band)), LoadType::Band);
    EXPECT_EQ(loadTypeFromString(loadTypeToString(LoadType::None)), LoadType::None);
    EXPECT_EQ(loadTypeFromString("nonsense"), LoadType::External);
}

TEST_F(SetTest, EffectiveRestSeconds_FallsBackToTheExerciseDefault)
{
    Set set(10, 80.0);

    EXPECT_EQ(set.effectiveRestSeconds(120), 120);
}

TEST_F(SetTest, EffectiveRestSeconds_PrefersTheOverrideIncludingZero)
{
    Set set = Set::createDuration(20);
    set.setRestSecondsOverride(10);
    EXPECT_EQ(set.effectiveRestSeconds(120), 10);

    set.setRestSecondsOverride(0);
    EXPECT_EQ(set.effectiveRestSeconds(120), 0);
}
