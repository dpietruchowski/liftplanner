#include <QSignalSpy>
#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"

// =============================================================================
// Active workout: navigation, set completion, duplicate/remove sets
// =============================================================================

class ActiveWorkoutTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        app.plannedWorkoutViewModel().importFromJson(TestData::THREE_WORKOUTS_JSON);
        app.plannedWorkoutViewModel().loadAll();
    }

    void startFirstWorkout()
    {
        auto* first = app.plannedWorkoutViewModel().workouts().first();
        app.activeWorkoutViewModel().startWorkout(first);
    }

    TestApplication app;
};

TEST_F(ActiveWorkoutTest, InitialState_NoActiveWorkout)
{
    TestApplication freshApp;
    auto& vm = freshApp.activeWorkoutViewModel();

    EXPECT_FALSE(vm.isActive());
    EXPECT_EQ(vm.currentWorkout(), nullptr);
    EXPECT_EQ(vm.currentExercise(), nullptr);
    EXPECT_EQ(vm.currentSet(), nullptr);
}

TEST_F(ActiveWorkoutTest, StartWorkout_SetsActiveState)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    EXPECT_TRUE(vm.isActive());
    ASSERT_NE(vm.currentWorkout(), nullptr);
    EXPECT_EQ(vm.currentWorkout()->name(), "Push Day");
}

TEST_F(ActiveWorkoutTest, StartWorkout_PointsToFirstExerciseAndSet)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    ASSERT_NE(vm.currentExercise(), nullptr);
    EXPECT_EQ(vm.currentExercise()->name(), "Bench Press");

    ASSERT_NE(vm.currentSet(), nullptr);
    EXPECT_EQ(vm.currentSet()->repetitions(), 10);
    EXPECT_EQ(vm.currentSet()->weight(), 60);
}

TEST_F(ActiveWorkoutTest, CompleteCurrentSet_MarksAsCompleted)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    auto* firstSet = vm.currentSet();
    EXPECT_FALSE(firstSet->completed());

    vm.completeCurrentSet();

    EXPECT_TRUE(firstSet->completed());
}

TEST_F(ActiveWorkoutTest, CompleteCurrentSet_NavigatesToNext)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    // First set: 10x60
    EXPECT_EQ(vm.currentSet()->repetitions(), 10);
    EXPECT_EQ(vm.currentSet()->weight(), 60);

    vm.completeCurrentSet();

    // Second set: 8x70
    ASSERT_NE(vm.currentSet(), nullptr);
    EXPECT_EQ(vm.currentSet()->repetitions(), 8);
    EXPECT_EQ(vm.currentSet()->weight(), 70);
}

TEST_F(ActiveWorkoutTest, NavigateToNext_MovesToNextExercise)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    // Bench Press has 3 sets, complete all to move to next exercise
    vm.completeCurrentSet();  // set 1
    vm.completeCurrentSet();  // set 2
    vm.completeCurrentSet();  // set 3

    // Should now be at Overhead Press
    ASSERT_NE(vm.currentExercise(), nullptr);
    EXPECT_EQ(vm.currentExercise()->name(), "Overhead Press");
    ASSERT_NE(vm.currentSet(), nullptr);
    EXPECT_EQ(vm.currentSet()->repetitions(), 10);
    EXPECT_EQ(vm.currentSet()->weight(), 30);
}

TEST_F(ActiveWorkoutTest, NavigateToPrevious_GoesBack)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    vm.completeCurrentSet();  // move to set 2
    EXPECT_EQ(vm.currentSet()->repetitions(), 8);

    vm.navigateToPrevious();
    EXPECT_EQ(vm.currentSet()->repetitions(), 10);
}

TEST_F(ActiveWorkoutTest, NavigateToPrevious_CrossesExerciseBoundary)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    // Navigate past all Bench Press sets
    vm.completeCurrentSet();
    vm.completeCurrentSet();
    vm.completeCurrentSet();
    EXPECT_EQ(vm.currentExercise()->name(), "Overhead Press");

    vm.navigateToPrevious();

    EXPECT_EQ(vm.currentExercise()->name(), "Bench Press");
    // Should be at last set of Bench Press
    EXPECT_EQ(vm.currentSet()->weight(), 80);
}

TEST_F(ActiveWorkoutTest, EndWorkout_ClearsState)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    QSignalSpy completedSpy(&vm, &ActiveWorkoutViewModel::workoutCompleted);
    vm.endWorkout();

    EXPECT_FALSE(vm.isActive());
    EXPECT_EQ(vm.currentWorkout(), nullptr);
    EXPECT_EQ(vm.currentExercise(), nullptr);
    EXPECT_EQ(vm.currentSet(), nullptr);
    EXPECT_EQ(completedSpy.count(), 1);
}

TEST_F(ActiveWorkoutTest, StartWorkout_NullWorkout_EmitsError)
{
    auto& vm = app.activeWorkoutViewModel();

    QSignalSpy errorSpy(&vm, &ActiveWorkoutViewModel::errorOccurred);
    vm.startWorkout(nullptr);

    EXPECT_GE(errorSpy.count(), 1);
    EXPECT_FALSE(vm.isActive());
}

TEST_F(ActiveWorkoutTest, DuplicateSet_AddsNewSet)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    auto* exercise = vm.currentExercise();
    int originalSetCount = exercise->sets().size();

    vm.duplicateSet(vm.currentSet());

    EXPECT_EQ(exercise->sets().size(), originalSetCount + 1);
}

TEST_F(ActiveWorkoutTest, DuplicateSet_CopiesValues)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    auto* originalSet = vm.currentSet();
    int expectedReps = originalSet->repetitions();
    int expectedWeight = originalSet->weight();

    vm.duplicateSet(originalSet);

    // Current set should now be the duplicated one
    EXPECT_EQ(vm.currentSet()->repetitions(), expectedReps);
    EXPECT_EQ(vm.currentSet()->weight(), expectedWeight);
    EXPECT_FALSE(vm.currentSet()->completed());
}

TEST_F(ActiveWorkoutTest, RemoveSet_RemovesFromExercise)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    auto* exercise = vm.currentExercise();
    int originalSetCount = exercise->sets().size();

    auto* setToRemove = vm.currentSet();
    vm.removeSet(setToRemove);

    EXPECT_EQ(exercise->sets().size(), originalSetCount - 1);
}

TEST_F(ActiveWorkoutTest, CompleteAllSets_WorkoutStillActive)
{
    auto& vm = app.activeWorkoutViewModel();
    startFirstWorkout();

    // Complete all sets by iterating exercises and sets
    for (auto* exercise : vm.currentWorkout()->exercises())
    {
        for (int i = 0; i < exercise->sets().size(); ++i)
        {
            vm.completeCurrentSet();
        }
    }

    // Workout is still active until explicitly ended
    EXPECT_TRUE(vm.isActive());
    EXPECT_NE(vm.currentWorkout(), nullptr);
}

// =============================================================================
// Active workout with empty workout (no exercises)
// =============================================================================

class ActiveWorkoutEmptyTest : public ::testing::Test
{
protected:
    TestApplication app;
};

TEST_F(ActiveWorkoutEmptyTest, StartEmptyWorkout_NoExerciseOrSet)
{
    app.plannedWorkoutViewModel().importFromJson(TestData::EMPTY_WORKOUT_JSON);
    app.plannedWorkoutViewModel().loadAll();

    auto& vm = app.activeWorkoutViewModel();
    vm.startWorkout(app.plannedWorkoutViewModel().workouts().first());

    EXPECT_TRUE(vm.isActive());
    EXPECT_NE(vm.currentWorkout(), nullptr);
    EXPECT_EQ(vm.currentExercise(), nullptr);
    EXPECT_EQ(vm.currentSet(), nullptr);
}
