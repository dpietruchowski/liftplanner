#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"

// =============================================================================
// Interval sequencer: a timed set counts its work down, completes itself,
// rests, and rolls into the next round without the user touching the phone
// =============================================================================

class IntervalWorkoutTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        app.setCurrentDateTime(QDateTime(QDate(2025, 3, 4), QTime(18, 0, 0)));
        start(TestData::INTERVAL_WORKOUT_JSON);
    }

    void start(const QString& json)
    {
        app.plannedWorkoutViewModel().importFromJson(json);
        app.plannedWorkoutViewModel().loadAll();
        app.drain();
        app.activeWorkoutViewModel().startWorkout(app.plannedWorkoutViewModel().workouts().first());
    }

    ActiveWorkoutViewModel& vm() { return app.activeWorkoutViewModel(); }
    WorkoutTimer& timer() { return *vm().timer(); }

    void elapse(int seconds)
    {
        app.advanceSeconds(seconds);
        timer().tick();
    }

    SetModel* setOf(int exerciseIndex, int setIndex)
    {
        return vm().currentWorkout()->exercises()[exerciseIndex]->sets()[setIndex];
    }

    int completedRounds()
    {
        int count = 0;
        for (auto* set : vm().currentWorkout()->exercises()[0]->sets())
            count += set->completed() ? 1 : 0;
        return count;
    }

    TestApplication app;
};

TEST_F(IntervalWorkoutTest, StartingTheWorkTimerCountsDownTheFirstRound)
{
    vm().startWorkTimer();

    EXPECT_EQ(timer().phaseLabel(), "Work");
    EXPECT_EQ(timer().remainingSeconds(), 20);
    EXPECT_EQ(timer().totalSeconds(), 20);
    EXPECT_TRUE(timer().isRunning());

    elapse(5);
    EXPECT_EQ(timer().remainingSeconds(), 15);
    EXPECT_EQ(timer().remainingText(), "00:15");
}

TEST_F(IntervalWorkoutTest, ElapsedWorkCompletesTheSetAndOpensTheRest)
{
    vm().startWorkTimer();
    elapse(20);

    EXPECT_TRUE(setOf(0, 0)->completed());
    EXPECT_EQ(vm().currentSet(), setOf(0, 1));
    EXPECT_EQ(timer().phaseLabel(), "Rest");
    EXPECT_EQ(timer().remainingSeconds(), 10);
}

TEST_F(IntervalWorkoutTest, ElapsedRestStartsTheNextRoundByItself)
{
    vm().startWorkTimer();
    elapse(20);
    elapse(10);

    EXPECT_EQ(timer().phaseLabel(), "Work");
    EXPECT_EQ(timer().remainingSeconds(), 20);
    EXPECT_EQ(vm().currentSet(), setOf(0, 1));
}

TEST_F(IntervalWorkoutTest, FourRoundsRunToTheEndWithoutAnyInput)
{
    vm().startWorkTimer();

    for (int round = 0; round < 4; ++round)
    {
        elapse(20);
        elapse(10);
    }

    EXPECT_EQ(completedRounds(), 4);
    EXPECT_EQ(vm().currentExercise(), vm().currentWorkout()->exercises()[1]);
    EXPECT_EQ(timer().phaseLabel(), "Work");
    EXPECT_EQ(timer().remainingSeconds(), 45);
}

TEST_F(IntervalWorkoutTest, RestFollowsThePerSetOverrideNotTheExerciseDefault)
{
    vm().startWorkTimer();
    elapse(20);
    EXPECT_EQ(timer().remainingSeconds(), 10);

    for (int round = 1; round < 4; ++round)
    {
        elapse(10);
        elapse(20);
    }

    EXPECT_EQ(completedRounds(), 4);

    elapse(10);
    elapse(45);
    EXPECT_EQ(timer().phaseLabel(), "Rest");
    EXPECT_EQ(timer().remainingSeconds(), 30);
}

TEST_F(IntervalWorkoutTest, DoneMidRoundEndsTheWorkAndStartsTheRest)
{
    vm().startWorkTimer();
    elapse(5);

    vm().completeCurrentSet();

    EXPECT_TRUE(setOf(0, 0)->completed());
    EXPECT_EQ(timer().phaseLabel(), "Rest");
    EXPECT_EQ(timer().remainingSeconds(), 10);
}

TEST_F(IntervalWorkoutTest, PauseFreezesTheCountdownAndResumePicksItUp)
{
    vm().startWorkTimer();
    elapse(5);

    timer().pause();
    EXPECT_TRUE(timer().isPaused());
    EXPECT_EQ(timer().remainingSeconds(), 15);

    elapse(30);
    EXPECT_EQ(timer().remainingSeconds(), 15);
    EXPECT_EQ(timer().phaseLabel(), "Work");
    EXPECT_FALSE(setOf(0, 0)->completed());

    timer().resume();
    EXPECT_FALSE(timer().isPaused());

    elapse(15);
    EXPECT_TRUE(setOf(0, 0)->completed());
    EXPECT_EQ(timer().phaseLabel(), "Rest");
}

TEST_F(IntervalWorkoutTest, AddingAndTrimmingSecondsMovesTheDeadline)
{
    vm().startWorkTimer();

    timer().addSeconds(10);
    EXPECT_EQ(timer().remainingSeconds(), 30);
    EXPECT_EQ(timer().totalSeconds(), 30);

    elapse(25);
    EXPECT_EQ(timer().remainingSeconds(), 5);

    timer().addSeconds(-60);
    EXPECT_EQ(timer().remainingSeconds(), 1);
    EXPECT_EQ(timer().phaseLabel(), "Work");
}

TEST_F(IntervalWorkoutTest, StoppingTheTimerLeavesTheRoundUntouched)
{
    vm().startWorkTimer();
    elapse(5);

    vm().toggleTimer();

    EXPECT_FALSE(timer().isRunning());
    EXPECT_EQ(timer().remainingSeconds(), 0);
    EXPECT_EQ(timer().phaseLabel(), "");
    EXPECT_FALSE(setOf(0, 0)->completed());
    EXPECT_EQ(vm().currentSet(), setOf(0, 0));
}

TEST_F(IntervalWorkoutTest, TheLastSetOfTheWorkoutDoesNotOpenARest)
{
    for (int i = 0; i < 6; ++i)
        vm().completeCurrentSet();

    EXPECT_TRUE(vm().currentWorkout()->isCompleted());
    EXPECT_FALSE(timer().isRunning());
}

TEST_F(IntervalWorkoutTest, CompletedRoundsSurviveAnAppRestartWithTheTimerIdle)
{
    vm().startWorkTimer();
    elapse(20);
    elapse(10);
    elapse(20);
    app.drain();

    TestApplication reopened;
    reopened.activeWorkoutViewModel().loadCurrentWorkout();

    auto& reloaded = reopened.activeWorkoutViewModel();
    auto* rounds = reloaded.currentWorkout()->exercises()[0];

    EXPECT_TRUE(rounds->sets()[0]->completed());
    EXPECT_TRUE(rounds->sets()[1]->completed());
    EXPECT_FALSE(rounds->sets()[2]->completed());
    EXPECT_EQ(reloaded.currentSet(), rounds->sets()[2]);
    EXPECT_FALSE(reloaded.timer()->isRunning());

    reloaded.startWorkTimer();
    EXPECT_EQ(reloaded.timer()->phaseLabel(), "Work");
    EXPECT_EQ(reloaded.timer()->remainingSeconds(), 20);
}

// =============================================================================
// Weighted work keeps the old behaviour: rest only, never an auto-started round
// =============================================================================

class WeightedTimerTest : public IntervalWorkoutTest
{
protected:
    void SetUp() override
    {
        app.setCurrentDateTime(QDateTime(QDate(2025, 3, 4), QTime(18, 0, 0)));
        start(TestData::MIXED_WORKOUT_JSON);
    }
};

TEST_F(WeightedTimerTest, CompletingAWeightedSetRestsButNeverAutoStartsWork)
{
    vm().completeCurrentSet();

    EXPECT_EQ(timer().phaseLabel(), "Rest");
    EXPECT_EQ(timer().remainingSeconds(), 120);

    elapse(120);
    EXPECT_FALSE(timer().isRunning());
    EXPECT_EQ(vm().currentSet(), setOf(0, 1));
}

TEST_F(WeightedTimerTest, TheTimerButtonStartsARestForAWeightedSet)
{
    vm().toggleTimer();

    EXPECT_EQ(timer().phaseLabel(), "Rest");
    EXPECT_EQ(timer().remainingSeconds(), 120);
}
