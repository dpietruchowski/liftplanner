#include <gtest/gtest.h>

#include "async/timeprovider.h"
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
    void importWorkouts(const QString& json)
    {
        app.plannedWorkoutViewModel().importFromJson(json);
        app.plannedWorkoutViewModel().loadAll();
        app.drain();
    }

    void completeAllSets(ActiveWorkoutViewModel& vm)
    {
        if (!vm.currentWorkout())
            return;

        for (auto* exercise : vm.currentWorkout()->exercises())
        {
            for (int i = 0; i < exercise->sets().size(); ++i)
            {
                vm.completeCurrentSet();
            }
        }
    }

    ActiveWorkoutViewModel& startFirstPlannedWorkout(const QString& json)
    {
        importWorkouts(json);
        auto& active = app.activeWorkoutViewModel();
        active.startWorkout(app.plannedWorkoutViewModel().workouts().first());
        return active;
    }

    QList<WorkoutModel*> reloadHistory()
    {
        app.workoutHistoryViewModel().loadAllWorkouts();
        app.drain();
        return app.workoutHistoryViewModel().workouts();
    }

    QList<WorkoutModel*> reloadPlanned()
    {
        app.plannedWorkoutViewModel().loadAll();
        app.drain();
        return app.plannedWorkoutViewModel().workouts();
    }

    static int tickedSetsOf(const Workout& workout)
    {
        int done = 0;
        for (const Exercise& exercise : workout.exercises())
            for (const Set& set : exercise.sets())
                done += set.completed() ? 1 : 0;
        return done;
    }

    TestApplication app;
};

// --- finishing a workout that was not carried to the end ---

TEST_F(WorkoutLifecycleTest, SetCountsReportTheProgressOfTheSession)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);

    EXPECT_EQ(active.totalSetCount(), 2);
    EXPECT_EQ(active.completedSetCount(), 0);

    active.completeCurrentSet();

    EXPECT_EQ(active.completedSetCount(), 1);
    EXPECT_EQ(active.totalSetCount(), 2);
}

TEST_F(WorkoutLifecycleTest, SetCountsAreZeroWithoutASession)
{
    auto& active = app.activeWorkoutViewModel();

    EXPECT_EQ(active.completedSetCount(), 0);
    EXPECT_EQ(active.totalSetCount(), 0);
    EXPECT_FALSE(active.hasAnythingToRecord());
}

TEST_F(WorkoutLifecycleTest, ASessionIsWorthRecordingOnceTheFirstSetIsTickedOff)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);

    EXPECT_FALSE(active.hasAnythingToRecord());

    active.completeCurrentSet();

    EXPECT_TRUE(active.hasAnythingToRecord());
}

TEST_F(WorkoutLifecycleTest, AHalfDoneWorkoutCanBeEndedAndReachesHistory)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();
    ASSERT_EQ(active.completedSetCount(), 1);

    active.endWorkout();

    EXPECT_FALSE(active.isActive());
    EXPECT_EQ(active.currentWorkout(), nullptr);

    const auto history = reloadHistory();
    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history.first()->name(), "Full Body");
    EXPECT_EQ(history.first()->statusString(), "Ended");
    EXPECT_EQ(history.first()->startedTime().date(), TimeProvider::instance().currentDate());
    EXPECT_EQ(history.first()->endedTime().date(), TimeProvider::instance().currentDate());
}

TEST_F(WorkoutLifecycleTest, TheSetsTheLifterSkippedStayUnticked)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();

    active.endWorkout();

    const auto history = reloadHistory();
    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(tickedSetsOf(history.first()->toEntity()), 1);
}

TEST_F(WorkoutLifecycleTest, OnlyTheTickedSetsOfAHalfDoneSessionCountTowardsVolume)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();
    active.endWorkout();

    reloadHistory();

    // Squat 5x100 is ticked off, Bench Press 5x80 is not.
    QString volume;
    for (const QVariant& tile : app.workoutHistoryViewModel().recentTotals())
    {
        const QVariantMap map = tile.toMap();
        if (map["label"].toString() == "volume")
            volume = map["value"].toString();
    }

    EXPECT_EQ(volume, "500 kg");
}

TEST_F(WorkoutLifecycleTest, EndingLeavesNoSessionForTheNextStartToAskAbout)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();

    active.endWorkout();

    EXPECT_EQ(active.currentWorkout(), nullptr);
}

// --- a session in which nothing was ticked off goes back to the plan ---

TEST_F(WorkoutLifecycleTest, DiscardingASessionReturnsItToThePlannedList)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    ASSERT_EQ(active.completedSetCount(), 0);

    active.discardWorkout();

    EXPECT_FALSE(active.isActive());
    EXPECT_EQ(active.currentWorkout(), nullptr);

    EXPECT_TRUE(reloadHistory().isEmpty());

    const auto planned = reloadPlanned();
    ASSERT_EQ(planned.size(), 1);
    EXPECT_EQ(planned.first()->name(), "Full Body");
    EXPECT_EQ(planned.first()->statusString(), "Planned");
}

TEST_F(WorkoutLifecycleTest, ADiscardedSessionAddsNothingToTheStats)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);

    active.discardWorkout();
    reloadHistory();

    EXPECT_TRUE(app.workoutHistoryViewModel().recentTotals().isEmpty());
    EXPECT_TRUE(app.workoutHistoryViewModel().topExercises().isEmpty());
}

TEST_F(WorkoutLifecycleTest, ADiscardedSessionKeepsItsExercisesForTheNextAttempt)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);

    active.discardWorkout();

    const auto planned = reloadPlanned();
    ASSERT_EQ(planned.size(), 1);
    const Workout entity = planned.first()->toEntity();
    EXPECT_EQ(entity.exercises().size(), 2u);
    EXPECT_EQ(entity.totalSets(), 2);
    EXPECT_EQ(tickedSetsOf(entity), 0);
}

TEST_F(WorkoutLifecycleTest, ADiscardedSessionIsBackOnThePlannedListWithoutAskingForAReload)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    app.drain();
    ASSERT_TRUE(reloadPlanned().isEmpty());

    active.discardWorkout();
    app.drain();

    const auto planned = app.plannedWorkoutViewModel().workouts();
    ASSERT_EQ(planned.size(), 1);
    EXPECT_EQ(planned.first()->name(), "Full Body");
}

TEST_F(WorkoutLifecycleTest, AnEndedSessionIsInTheHistoryWithoutAskingForAReload)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();

    active.endWorkout();
    app.drain();

    const auto history = app.workoutHistoryViewModel().workouts();
    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history.first()->name(), "Full Body");
}

// --- the summary the lifter sees the moment the session ends ---

TEST_F(WorkoutLifecycleTest, TheSummaryOfAFinishedSessionReportsEverySetAndItsVolume)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    completeAllSets(active);

    active.endWorkout();

    const QVariantMap summary = active.lastSessionSummary();
    EXPECT_EQ(summary["name"].toString(), "Full Body");
    EXPECT_EQ(summary["completedSets"].toInt(), 2);
    EXPECT_EQ(summary["plannedSets"].toInt(), 2);
    EXPECT_EQ(summary["setsText"].toString(), "2/2");
    EXPECT_EQ(summary["volumeText"].toString(), "900 kg");
}

TEST_F(WorkoutLifecycleTest, TheSummaryOfAnAbandonedSessionReportsOnlyWhatWasTickedOff)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();

    active.endWorkout();

    const QVariantMap summary = active.lastSessionSummary();
    EXPECT_EQ(summary["completedSets"].toInt(), 1);
    EXPECT_EQ(summary["plannedSets"].toInt(), 2);
    EXPECT_EQ(summary["setsText"].toString(), "1/2");
    EXPECT_EQ(summary["volumeText"].toString(), "500 kg");
}

TEST_F(WorkoutLifecycleTest, TheSummaryAgreesWithTheVolumeOnTheHomeScreen)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();
    active.endWorkout();

    reloadHistory();

    QString homeVolume;
    for (const QVariant& tile : app.workoutHistoryViewModel().recentTotals())
    {
        const QVariantMap map = tile.toMap();
        if (map["label"].toString() == "volume")
            homeVolume = map["value"].toString();
    }

    EXPECT_EQ(active.lastSessionSummary()["volumeText"].toString(), homeVolume);
}

TEST_F(WorkoutLifecycleTest, TheSummaryReportsHowLongTheLifterWasUnderTheBar)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();
    app.advanceSeconds(3900);

    active.endWorkout();

    const QVariantMap summary = active.lastSessionSummary();
    EXPECT_EQ(summary["durationSeconds"].toLongLong(), 3900);
    EXPECT_EQ(summary["durationText"].toString(), "1h 05m");
}

TEST_F(WorkoutLifecycleTest, ADiscardedSessionProducesNoSummaryToShow)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);

    active.discardWorkout();

    EXPECT_TRUE(active.lastSessionSummary().isEmpty());
}

// --- doing a workout from the history a second time ---

TEST_F(WorkoutLifecycleTest, RepeatingAFinishedSessionPutsItBackOnThePlannedList)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    completeAllSets(active);
    active.endWorkout();
    app.drain();

    const auto history = reloadHistory();
    ASSERT_EQ(history.size(), 1);
    ASSERT_TRUE(reloadPlanned().isEmpty());

    app.plannedWorkoutViewModel().repeatWorkout(history.first());
    app.drain();

    const auto planned = app.plannedWorkoutViewModel().workouts();
    ASSERT_EQ(planned.size(), 1);
    EXPECT_EQ(planned.first()->name(), "Full Body");
    EXPECT_EQ(planned.first()->statusString(), "Planned");
}

TEST_F(WorkoutLifecycleTest, TheRepeatedWorkoutCarriesTheSameExercisesAndWeights)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    completeAllSets(active);
    active.endWorkout();
    app.drain();

    app.plannedWorkoutViewModel().repeatWorkout(reloadHistory().first());
    app.drain();

    const Workout copy = reloadPlanned().first()->toEntity();
    ASSERT_EQ(copy.exercises().size(), 2u);
    EXPECT_EQ(copy.exercises()[0].name(), "Squat");
    EXPECT_EQ(copy.exercises()[1].name(), "Bench Press");
    ASSERT_EQ(copy.exercises()[0].sets().size(), 1u);
    EXPECT_DOUBLE_EQ(copy.exercises()[0].sets().front().weight(), 100.0);
    EXPECT_DOUBLE_EQ(copy.exercises()[1].sets().front().weight(), 80.0);
}

TEST_F(WorkoutLifecycleTest, TheRepeatedWorkoutArrivesWithoutASingleTick)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    completeAllSets(active);
    active.endWorkout();
    app.drain();

    app.plannedWorkoutViewModel().repeatWorkout(reloadHistory().first());
    app.drain();

    EXPECT_EQ(tickedSetsOf(reloadPlanned().first()->toEntity()), 0);
}

TEST_F(WorkoutLifecycleTest, RepeatingAddsNothingToTheStatsUntilItIsActuallyDone)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();
    active.endWorkout();
    app.drain();

    const QVariantList before = app.workoutHistoryViewModel().recentTotals();

    app.plannedWorkoutViewModel().repeatWorkout(reloadHistory().first());
    app.drain();
    reloadHistory();

    EXPECT_EQ(app.workoutHistoryViewModel().recentTotals(), before);
}

TEST_F(WorkoutLifecycleTest, RepeatingLeavesTheOriginalEntryInTheHistoryUntouched)
{
    auto& active = startFirstPlannedWorkout(TestData::SINGLE_WORKOUT_JSON);
    active.completeCurrentSet();
    active.endWorkout();
    app.drain();

    auto* original = reloadHistory().first();
    const int originalId = original->id();

    app.plannedWorkoutViewModel().repeatWorkout(original);
    app.drain();

    const auto history = reloadHistory();
    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history.first()->id(), originalId);
    EXPECT_EQ(history.first()->statusString(), "Ended");
    EXPECT_EQ(tickedSetsOf(history.first()->toEntity()), 1);

    EXPECT_NE(reloadPlanned().first()->id(), originalId);
}

TEST_F(WorkoutLifecycleTest, DiscardingWithoutASessionDoesNothing)
{
    auto& active = app.activeWorkoutViewModel();

    active.discardWorkout();

    EXPECT_EQ(active.currentWorkout(), nullptr);
    EXPECT_TRUE(reloadHistory().isEmpty());
}

TEST_F(WorkoutLifecycleTest, ImportPlanStartAndEnd_AppearsInHistory)
{
    importWorkouts(TestData::THREE_WORKOUTS_JSON);

    auto& planned = app.plannedWorkoutViewModel();
    ASSERT_EQ(planned.workouts().size(), 3);

    auto& active = app.activeWorkoutViewModel();
    active.startWorkout(planned.workouts().first());

    EXPECT_TRUE(active.isActive());
    EXPECT_NE(active.currentWorkout(), nullptr);
    EXPECT_EQ(active.currentWorkout()->name(), "Push Day");
    EXPECT_TRUE(active.currentWorkout()->startedTime().isValid());

    active.endWorkout();

    EXPECT_FALSE(active.isActive());
    EXPECT_EQ(active.currentWorkout(), nullptr);

    app.workoutHistoryViewModel().loadAllWorkouts();
    app.drain();
    auto& history = app.workoutHistoryViewModel();
    ASSERT_EQ(history.workouts().size(), 1);
    EXPECT_EQ(history.workouts().first()->name(), "Push Day");
    EXPECT_TRUE(history.workouts().first()->endedTime().isValid());
}

TEST_F(WorkoutLifecycleTest, CompletedWorkout_HasCorrectStatus)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto& planned = app.plannedWorkoutViewModel();
    auto& active = app.activeWorkoutViewModel();

    active.startWorkout(planned.workouts().first());
    EXPECT_EQ(active.currentWorkout()->statusString(), "Started");

    active.endWorkout();

    app.workoutHistoryViewModel().loadAllWorkouts();
    app.drain();
    auto* last = app.workoutHistoryViewModel().lastWorkout();
    ASSERT_NE(last, nullptr);
    EXPECT_EQ(last->statusString(), "Ended");
}

TEST_F(WorkoutLifecycleTest, CompletedWorkout_HasAllTimestamps)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto& active = app.activeWorkoutViewModel();
    active.startWorkout(app.plannedWorkoutViewModel().workouts().first());
    active.endWorkout();

    app.workoutHistoryViewModel().loadAllWorkouts();
    app.drain();
    auto* last = app.workoutHistoryViewModel().lastWorkout();
    ASSERT_NE(last, nullptr);

    EXPECT_TRUE(last->startedTime().isValid());
    EXPECT_TRUE(last->endedTime().isValid());
    EXPECT_GE(last->endedTime(), last->startedTime());
}

TEST_F(WorkoutLifecycleTest, MultipleWorkouts_AllAppearInHistory)
{
    importWorkouts(TestData::THREE_WORKOUTS_JSON);

    auto& planned = app.plannedWorkoutViewModel();
    auto& active = app.activeWorkoutViewModel();

    // Start and end first workout
    active.startWorkout(planned.workouts().at(0));
    active.endWorkout();

    // Start and end second workout
    active.startWorkout(planned.workouts().at(1));
    active.endWorkout();

    app.workoutHistoryViewModel().loadAllWorkouts();
    app.drain();
    EXPECT_EQ(app.workoutHistoryViewModel().workouts().size(), 2);
}

TEST_F(WorkoutLifecycleTest, StartWorkout_ClonesNotModifiesOriginal)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto& planned = app.plannedWorkoutViewModel();
    auto* originalWorkout = planned.workouts().first();
    QString originalName = originalWorkout->name();
    int originalId = originalWorkout->id();

    auto& active = app.activeWorkoutViewModel();
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

    auto& active = app.activeWorkoutViewModel();
    active.startWorkout(app.plannedWorkoutViewModel().workouts().first());

    ASSERT_NE(active.currentExercise(), nullptr);
    ASSERT_NE(active.currentSet(), nullptr);

    // Complete all sets by navigating through the workout
    completeAllSets(active);

    active.endWorkout();

    app.workoutHistoryViewModel().loadAllWorkouts();
    app.drain();
    auto* last = app.workoutHistoryViewModel().lastWorkout();
    ASSERT_NE(last, nullptr);
    EXPECT_EQ(last->name(), "Full Body");
}

TEST_F(WorkoutLifecycleTest, DeleteFromHistory)
{
    importWorkouts(TestData::SINGLE_WORKOUT_JSON);

    auto& active = app.activeWorkoutViewModel();
    active.startWorkout(app.plannedWorkoutViewModel().workouts().first());
    active.endWorkout();

    auto& history = app.workoutHistoryViewModel();
    history.loadAllWorkouts();
    app.drain();
    ASSERT_EQ(history.workouts().size(), 1);

    history.deleteWorkout(history.workouts().first());
    app.drain();
    EXPECT_EQ(history.workouts().size(), 0);
}
