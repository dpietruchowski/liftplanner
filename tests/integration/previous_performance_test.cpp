#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"

class PreviousPerformanceTest : public ::testing::Test
{
protected:
    void importAndStartFirstWorkout()
    {
        app.plannedWorkoutViewModel().importFromJson(TestData::THREE_WORKOUTS_JSON);
        app.plannedWorkoutViewModel().loadAll();
        app.drain();
        app.activeWorkoutViewModel().startWorkout(app.plannedWorkoutViewModel().workouts().first());
        app.drain();
    }

    void TearDown() override
    {
        app.activeWorkoutViewModel().endWorkout();
        app.drain();
    }

    TestApplication app;
};

TEST_F(PreviousPerformanceTest, TheFirstSessionHasNothingToCompareWith)
{
    importAndStartFirstWorkout();

    for (auto* exercise : app.activeWorkoutViewModel().currentWorkout()->exercises())
    {
        EXPECT_TRUE(exercise->previousSummary().isEmpty());
        EXPECT_FALSE(exercise->previousDate().isValid());
    }
}

TEST_F(PreviousPerformanceTest, TheNextSessionShowsWhatWasCompletedLastTime)
{
    importAndStartFirstWorkout();
    auto& vm = app.activeWorkoutViewModel();
    const QDateTime firstSession = vm.currentWorkout()->startedTime();
    vm.completeCurrentSet();
    vm.completeCurrentSet();
    vm.endWorkout();
    app.drain();

    app.advanceDay();
    importAndStartFirstWorkout();

    const auto exercises = vm.currentWorkout()->exercises();
    EXPECT_EQ(exercises[0]->previousDate(), firstSession);
    EXPECT_TRUE(exercises[0]->previousSummary().contains("60"));
    EXPECT_TRUE(exercises[0]->previousSummary().contains("70"));
    EXPECT_FALSE(exercises[0]->previousSummary().contains("80"));
    EXPECT_TRUE(exercises[1]->previousSummary().isEmpty());
}
