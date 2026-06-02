#include <QSignalSpy>
#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

// =============================================================================
// Planned workout import, listing, re-import scenarios
// =============================================================================

class PlannedWorkoutTest : public ::testing::Test
{
protected:
    TestApplication app;
};

TEST_F(PlannedWorkoutTest, EmptyState_NoWorkouts)
{
    auto& vm = app.plannedWorkoutViewModel();
    vm.loadAll();

    EXPECT_TRUE(vm.workouts().isEmpty());
    EXPECT_EQ(vm.nextWorkout(), nullptr);
}

TEST_F(PlannedWorkoutTest, ImportFromJson_LoadsWorkouts)
{
    auto& vm = app.plannedWorkoutViewModel();
    vm.importFromJson(TestData::THREE_WORKOUTS_JSON);
    vm.loadAll();

    EXPECT_EQ(vm.workouts().size(), 3);
    EXPECT_EQ(vm.workouts().at(0)->name(), "Push Day");
    EXPECT_EQ(vm.workouts().at(1)->name(), "Pull Day");
    EXPECT_EQ(vm.workouts().at(2)->name(), "Leg Day");
}

TEST_F(PlannedWorkoutTest, ImportFromJson_SetsPlannedTime)
{
    auto& vm = app.plannedWorkoutViewModel();
    vm.importFromJson(TestData::THREE_WORKOUTS_JSON);
    vm.loadAll();

    for (auto* w : vm.workouts())
    {
        EXPECT_TRUE(w->plannedTime().isValid());
    }

    // Should be ordered by planned time ascending
    EXPECT_LT(vm.workouts().at(0)->plannedTime(), vm.workouts().at(1)->plannedTime());
    EXPECT_LT(vm.workouts().at(1)->plannedTime(), vm.workouts().at(2)->plannedTime());
}

TEST_F(PlannedWorkoutTest, NextWorkout_ReturnsFirst)
{
    auto& vm = app.plannedWorkoutViewModel();
    vm.importFromJson(TestData::THREE_WORKOUTS_JSON);
    vm.loadAll();

    auto* next = vm.nextWorkout();
    ASSERT_NE(next, nullptr);
    EXPECT_EQ(next->name(), "Push Day");
}

TEST_F(PlannedWorkoutTest, ReImport_ReplacesAll)
{
    auto& vm = app.plannedWorkoutViewModel();

    vm.importFromJson(TestData::THREE_WORKOUTS_JSON);
    vm.loadAll();
    EXPECT_EQ(vm.workouts().size(), 3);

    vm.importFromJson(TestData::SINGLE_WORKOUT_JSON);
    vm.loadAll();

    EXPECT_EQ(vm.workouts().size(), 1);
    EXPECT_EQ(vm.workouts().first()->name(), "Full Body");
}

TEST_F(PlannedWorkoutTest, ImportedWorkouts_HaveExercisesAndSets)
{
    auto& vm = app.plannedWorkoutViewModel();
    vm.importFromJson(TestData::THREE_WORKOUTS_JSON);
    vm.loadAll();

    auto* pushDay = vm.workouts().at(0);
    auto exercises = pushDay->exercises();
    ASSERT_EQ(exercises.size(), 2);

    EXPECT_EQ(exercises.at(0)->name(), "Bench Press");
    EXPECT_EQ(exercises.at(0)->restSeconds(), 120);
    EXPECT_EQ(exercises.at(0)->sets().size(), 3);
    EXPECT_EQ(exercises.at(0)->sets().at(0)->repetitions(), 10);
    EXPECT_EQ(exercises.at(0)->sets().at(0)->weight(), 60);

    EXPECT_EQ(exercises.at(1)->name(), "Overhead Press");
    EXPECT_EQ(exercises.at(1)->sets().size(), 2);
}

TEST_F(PlannedWorkoutTest, ImportInvalidJson_EmitsError)
{
    auto& vm = app.plannedWorkoutViewModel();

    QSignalSpy errorSpy(&vm, &PlannedWorkoutViewModel::errorOccurred);
    vm.importFromJson("not valid json");

    EXPECT_GE(errorSpy.count(), 1);
}

TEST_F(PlannedWorkoutTest, ImportEmptyArray_ClearsAll)
{
    auto& vm = app.plannedWorkoutViewModel();

    vm.importFromJson(TestData::THREE_WORKOUTS_JSON);
    vm.loadAll();
    EXPECT_EQ(vm.workouts().size(), 3);

    vm.importFromJson(R"({"user_profile": null, "workouts": []})");
    vm.loadAll();

    EXPECT_EQ(vm.workouts().size(), 0);
}

TEST_F(PlannedWorkoutTest, PlannedWorkouts_HaveStatusPlanned)
{
    auto& vm = app.plannedWorkoutViewModel();
    vm.importFromJson(TestData::THREE_WORKOUTS_JSON);
    vm.loadAll();

    for (auto* w : vm.workouts())
    {
        EXPECT_EQ(w->statusString(), "Planned");
    }
}

TEST_F(PlannedWorkoutTest, PlannedWorkouts_NotInHistory)
{
    auto& planned = app.plannedWorkoutViewModel();
    planned.importFromJson(TestData::THREE_WORKOUTS_JSON);
    planned.loadAll();

    auto& history = app.workoutHistoryViewModel();
    history.loadAllWorkouts();

    EXPECT_EQ(history.workouts().size(), 0);
}
