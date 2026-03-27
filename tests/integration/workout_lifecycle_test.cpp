#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

// =============================================================================
// Full workout lifecycle: import → start → complete → end → history
// =============================================================================

class WorkoutLifecycleTest : public ::testing::Test
{
protected:
    void importWorkouts(const QString &json)
    {
        app.plannedWorkoutViewModel().importFromJson(json);
        app.plannedWorkoutViewModel().loadAll();
    }

    void completeAllSets(ActiveWorkoutViewModel &vm)
    {
        if (!vm.currentWorkout())
            return;

        for (auto *exercise : vm.currentWorkout()->exercises())
        {
            for (int i = 0; i < exercise->sets().size(); ++i)
            {
                vm.completeCurrentSet();
            }
        }
    }

    TestApplication app;
};

TEST_F(WorkoutLifecycleTest, ImportPlanStartAndEnd_AppearsInHistory)
{
    importWorkouts(TestData::THREE_WORKOUTS_JSON);

    auto &planned = app.plannedWorkoutViewModel();
    ASSERT_EQ(planned.workouts().size(), 3);

    auto &active = app.activeWorkoutViewModel();
    active.startWorkout(planned.workouts().first());

    EXPECT_TRUE(active.isActive());
    EXPECT_NE(active.currentWorkout(), nullptr);
    EXPECT_EQ(active.currentWorkout()->name(), "Push Day");
    EXPECT_TRUE(active.currentWorkout()->startedTime().isValid());

    active.endWorkout();

    EXPECT_FALSE(active.isActive());
    EXPECT_EQ(active.currentWorkout(), nullptr);

    app.workoutHistoryViewModel().loadAllWorkouts();
    auto &history = app.workoutHistoryViewModel();
    ASSERT_EQ(history.workouts().size(), 1);
    EXPECT_EQ(history.workouts().first()->name(), "Push Day");
    EXPECT_TRUE(history.workouts().first()->endedTime().isValid());
}

TEST_F(WorkoutLifecycleTest, CompletedWorkout_HasCorrectStatus)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto &planned = app.plannedWorkoutViewModel();
    auto &active = app.activeWorkoutViewModel();

    active.startWorkout(planned.workouts().first());
    EXPECT_EQ(active.currentWorkout()->statusString(), "Started");

    active.endWorkout();

    app.workoutHistoryViewModel().loadAllWorkouts();
    auto *last = app.workoutHistoryViewModel().lastWorkout();
    ASSERT_NE(last, nullptr);
    EXPECT_EQ(last->statusString(), "Ended");
}

TEST_F(WorkoutLifecycleTest, CompletedWorkout_HasAllTimestamps)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto &active = app.activeWorkoutViewModel();
    active.startWorkout(app.plannedWorkoutViewModel().workouts().first());
    active.endWorkout();

    app.workoutHistoryViewModel().loadAllWorkouts();
    auto *last = app.workoutHistoryViewModel().lastWorkout();
    ASSERT_NE(last, nullptr);

    EXPECT_TRUE(last->startedTime().isValid());
    EXPECT_TRUE(last->endedTime().isValid());
    EXPECT_GE(last->endedTime(), last->startedTime());
}

TEST_F(WorkoutLifecycleTest, MultipleWorkouts_AllAppearInHistory)
{
    importWorkouts(TestData::THREE_WORKOUTS_JSON);

    auto &planned = app.plannedWorkoutViewModel();
    auto &active = app.activeWorkoutViewModel();

    // Start and end first workout
    active.startWorkout(planned.workouts().at(0));
    active.endWorkout();

    // Start and end second workout
    active.startWorkout(planned.workouts().at(1));
    active.endWorkout();

    app.workoutHistoryViewModel().loadAllWorkouts();
    EXPECT_EQ(app.workoutHistoryViewModel().workouts().size(), 2);
}

TEST_F(WorkoutLifecycleTest, StartWorkout_ClonesNotModifiesOriginal)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto &planned = app.plannedWorkoutViewModel();
    auto *originalWorkout = planned.workouts().first();
    QString originalName = originalWorkout->name();
    int originalId = originalWorkout->id();

    auto &active = app.activeWorkoutViewModel();
    active.startWorkout(originalWorkout);

    // Original planned workout should be unchanged
    EXPECT_EQ(originalWorkout->name(), originalName);
    EXPECT_EQ(originalWorkout->id(), originalId);

    // Active workout is a different object
    EXPECT_NE(active.currentWorkout(), originalWorkout);
    EXPECT_EQ(active.currentWorkout()->name(), originalName);
}

TEST_F(WorkoutLifecycleTest, CompleteAllSets_ThenEndWorkout)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto &active = app.activeWorkoutViewModel();
    active.startWorkout(app.plannedWorkoutViewModel().workouts().first());

    ASSERT_NE(active.currentExercise(), nullptr);
    ASSERT_NE(active.currentSet(), nullptr);

    // Complete all sets by navigating through the workout
    completeAllSets(active);

    active.endWorkout();

    app.workoutHistoryViewModel().loadAllWorkouts();
    auto *last = app.workoutHistoryViewModel().lastWorkout();
    ASSERT_NE(last, nullptr);
    EXPECT_EQ(last->name(), "Full Body");
}

TEST_F(WorkoutLifecycleTest, DeleteFromHistory)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto &active = app.activeWorkoutViewModel();
    active.startWorkout(app.plannedWorkoutViewModel().workouts().first());
    active.endWorkout();

    auto &history = app.workoutHistoryViewModel();
    history.loadAllWorkouts();
    ASSERT_EQ(history.workouts().size(), 1);

    history.deleteWorkout(history.workouts().first());
    EXPECT_EQ(history.workouts().size(), 0);
}
