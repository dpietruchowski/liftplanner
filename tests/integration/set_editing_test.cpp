#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"

// =============================================================================
// Editing a set: the step size and the editable fields follow the set's metric
// =============================================================================

class SetEditingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        app.plannedWorkoutViewModel().importFromJson(TestData::MIXED_WORKOUT_JSON);
        app.plannedWorkoutViewModel().loadAll();
        app.drain();
        app.activeWorkoutViewModel().startWorkout(app.plannedWorkoutViewModel().workouts().first());
    }

    SetModel* setOf(int exerciseIndex, int setIndex)
    {
        return app.activeWorkoutViewModel()
            .currentWorkout()
            ->exercises()[exerciseIndex]
            ->sets()[setIndex];
    }

    SetModel* benchSet() { return setOf(0, 0); }
    SetModel* pullupSet() { return setOf(1, 0); }
    SetModel* assistedPullupSet() { return setOf(1, 2); }
    SetModel* plankSet() { return setOf(2, 0); }
    SetModel* runSet() { return setOf(4, 0); }

    TestApplication app;
};

TEST_F(SetEditingTest, WeightedSet_PrimaryStepsReps_SecondaryStepsWeight)
{
    auto& vm = app.activeWorkoutViewModel();
    SetModel* set = benchSet();

    vm.adjustSetPrimary(set, 1);
    EXPECT_EQ(set->repetitions(), 11);

    vm.adjustSetPrimary(set, -2);
    EXPECT_EQ(set->repetitions(), 9);

    vm.adjustSetSecondary(set, 1);
    EXPECT_DOUBLE_EQ(set->weight(), 62.5);

    vm.adjustSetSecondary(set, -2);
    EXPECT_DOUBLE_EQ(set->weight(), 57.5);
}

TEST_F(SetEditingTest, TimedSet_PrimaryStepsFiveSeconds)
{
    auto& vm = app.activeWorkoutViewModel();
    SetModel* set = plankSet();

    EXPECT_EQ(set->durationSeconds(), 45);

    vm.adjustSetPrimary(set, 1);
    EXPECT_EQ(set->durationSeconds(), 50);

    vm.adjustSetPrimary(set, -3);
    EXPECT_EQ(set->durationSeconds(), 35);

    EXPECT_EQ(set->repetitions(), 0);
}

TEST_F(SetEditingTest, TimedSet_HasNoSecondaryField)
{
    auto& vm = app.activeWorkoutViewModel();
    SetModel* set = plankSet();

    EXPECT_FALSE(set->secondaryAdjustable());

    vm.adjustSetSecondary(set, 5);

    EXPECT_DOUBLE_EQ(set->weight(), 0.0);
    EXPECT_EQ(set->durationSeconds(), 45);
}

TEST_F(SetEditingTest, DistanceSet_PrimaryStepsHundredMeters_SecondaryStepsTime)
{
    auto& vm = app.activeWorkoutViewModel();
    SetModel* set = runSet();

    EXPECT_DOUBLE_EQ(set->distanceMeters(), 5000.0);
    EXPECT_EQ(set->durationSeconds(), 1440);

    vm.adjustSetPrimary(set, 2);
    EXPECT_DOUBLE_EQ(set->distanceMeters(), 5200.0);

    vm.adjustSetSecondary(set, -1);
    EXPECT_EQ(set->durationSeconds(), 1410);
}

TEST_F(SetEditingTest, BodyweightSet_HasNoLoadToAdjust)
{
    auto& vm = app.activeWorkoutViewModel();
    SetModel* set = pullupSet();

    EXPECT_FALSE(set->secondaryAdjustable());

    vm.adjustSetSecondary(set, 4);

    EXPECT_DOUBLE_EQ(set->weight(), 0.0);
    EXPECT_EQ(set->repetitions(), 8);
}

TEST_F(SetEditingTest, AssistedSet_AdjustsTheAssistance)
{
    auto& vm = app.activeWorkoutViewModel();
    SetModel* set = assistedPullupSet();

    EXPECT_TRUE(set->secondaryAdjustable());
    EXPECT_DOUBLE_EQ(set->weight(), 10.0);

    vm.adjustSetSecondary(set, 1);
    EXPECT_DOUBLE_EQ(set->weight(), 12.5);
}

TEST_F(SetEditingTest, Adjustments_NeverGoNegative)
{
    auto& vm = app.activeWorkoutViewModel();

    vm.adjustSetPrimary(benchSet(), -50);
    EXPECT_EQ(benchSet()->repetitions(), 0);

    vm.adjustSetSecondary(benchSet(), -50);
    EXPECT_DOUBLE_EQ(benchSet()->weight(), 0.0);

    vm.adjustSetPrimary(plankSet(), -50);
    EXPECT_EQ(plankSet()->durationSeconds(), 0);

    vm.adjustSetPrimary(runSet(), -100);
    EXPECT_DOUBLE_EQ(runSet()->distanceMeters(), 0.0);

    vm.adjustSetSecondary(runSet(), -100);
    EXPECT_EQ(runSet()->durationSeconds(), 0);
}

TEST_F(SetEditingTest, NullSetOrZeroSteps_AreIgnored)
{
    auto& vm = app.activeWorkoutViewModel();
    SetModel* set = benchSet();

    vm.adjustSetPrimary(nullptr, 1);
    vm.adjustSetSecondary(nullptr, 1);
    vm.adjustSetPrimary(set, 0);
    vm.adjustSetSecondary(set, 0);

    EXPECT_EQ(set->repetitions(), 10);
    EXPECT_DOUBLE_EQ(set->weight(), 60.0);
}

TEST_F(SetEditingTest, DisplayText_FollowsTheMetric)
{
    EXPECT_EQ(benchSet()->primaryText(), "10 reps");
    EXPECT_EQ(benchSet()->secondaryText(), "60 kg");

    EXPECT_EQ(pullupSet()->primaryText(), "8 reps");
    EXPECT_EQ(pullupSet()->secondaryText(), "BW");

    EXPECT_EQ(assistedPullupSet()->secondaryText(), "BW - 10 kg");

    EXPECT_EQ(plankSet()->primaryText(), "45 s");
    EXPECT_TRUE(plankSet()->secondaryText().isEmpty());

    EXPECT_EQ(runSet()->primaryText(), "5 km");
    EXPECT_EQ(runSet()->secondaryText(), "24:00");
}

TEST_F(SetEditingTest, DisplayText_UpdatesAfterAdjustment)
{
    auto& vm = app.activeWorkoutViewModel();

    vm.adjustSetPrimary(plankSet(), 3);
    EXPECT_EQ(plankSet()->primaryText(), "1:00");

    vm.adjustSetPrimary(runSet(), -10);
    EXPECT_EQ(runSet()->primaryText(), "4 km");
}

TEST_F(SetEditingTest, Adjustment_SurvivesTheSessionReload)
{
    auto& vm = app.activeWorkoutViewModel();

    vm.adjustSetPrimary(plankSet(), 3);
    vm.adjustSetSecondary(benchSet(), 2);
    app.drain();

    TestApplication reopened;
    reopened.activeWorkoutViewModel().loadCurrentWorkout();

    auto* reloadedPlank
        = reopened.activeWorkoutViewModel().currentWorkout()->exercises()[2]->sets()[0];
    auto* reloadedBench
        = reopened.activeWorkoutViewModel().currentWorkout()->exercises()[0]->sets()[0];

    EXPECT_EQ(reloadedPlank->durationSeconds(), 60);
    EXPECT_DOUBLE_EQ(reloadedBench->weight(), 65.0);
}
