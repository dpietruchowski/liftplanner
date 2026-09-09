#include <QSignalSpy>
#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "infrastructure/workout/workoutjson.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

// =============================================================================
// Importing plans that mix strength, bodyweight, timed, interval and cardio work
// =============================================================================

class MixedWorkoutImportTest : public ::testing::Test
{
protected:
    void importMixed()
    {
        app.plannedWorkoutViewModel().importFromJson(TestData::MIXED_WORKOUT_JSON);
        app.plannedWorkoutViewModel().loadAll();
        app.drain();
    }

    Workout importedWorkout()
    {
        return app.plannedWorkoutViewModel().workouts().first()->toEntity();
    }

    TestApplication app;
};

TEST_F(MixedWorkoutImportTest, Import_KeepsEveryExerciseKind)
{
    importMixed();

    const Workout workout = importedWorkout();

    ASSERT_EQ(workout.exercises().size(), 5u);
    EXPECT_EQ(workout.exercises()[0].kind(), ExerciseKind::Strength);
    EXPECT_EQ(workout.exercises()[1].kind(), ExerciseKind::Bodyweight);
    EXPECT_EQ(workout.exercises()[2].kind(), ExerciseKind::Mobility);
    EXPECT_EQ(workout.exercises()[3].kind(), ExerciseKind::Interval);
    EXPECT_EQ(workout.exercises()[4].kind(), ExerciseKind::Cardio);
}

TEST_F(MixedWorkoutImportTest, Import_ParsesBodyweightAndAssistedSets)
{
    importMixed();

    const Workout workout = importedWorkout();
    const Exercise& pullups = workout.exercises()[1];

    ASSERT_EQ(pullups.sets().size(), 3u);
    EXPECT_EQ(pullups.sets()[0].loadType(), LoadType::Bodyweight);
    EXPECT_EQ(pullups.sets()[0].repetitions(), 8);
    EXPECT_EQ(pullups.sets()[2].loadType(), LoadType::Assisted);
    EXPECT_DOUBLE_EQ(pullups.sets()[2].weight(), 10.0);
    EXPECT_DOUBLE_EQ(pullups.totalWeight(), 0.0);
    EXPECT_EQ(pullups.totalRepetitions(), 19);
}

TEST_F(MixedWorkoutImportTest, Import_ExpandsIntervalIntoRounds)
{
    importMixed();

    const Workout workout = importedWorkout();
    const Exercise& intervals = workout.exercises()[3];

    ASSERT_EQ(intervals.sets().size(), 8u);
    EXPECT_EQ(intervals.totalDurationSeconds(), 160);
    for (size_t i = 0; i < intervals.sets().size(); ++i)
    {
        EXPECT_EQ(intervals.sets()[i].metric(), SetMetric::Duration);
        EXPECT_EQ(intervals.restSecondsForSet(static_cast<int>(i)), 10);
    }
}

TEST_F(MixedWorkoutImportTest, Import_ParsesTimedAndDistanceSets)
{
    importMixed();

    const Workout workout = importedWorkout();

    const Exercise& plank = workout.exercises()[2];
    ASSERT_EQ(plank.sets().size(), 3u);
    EXPECT_EQ(plank.sets()[0].durationSeconds(), 45);
    EXPECT_EQ(plank.sets()[2].durationSeconds(), 180);
    EXPECT_EQ(plank.restSecondsForSet(0), 60);

    const Exercise& run = workout.exercises()[4];
    ASSERT_EQ(run.sets().size(), 1u);
    EXPECT_EQ(run.sets()[0].metric(), SetMetric::Distance);
    EXPECT_DOUBLE_EQ(run.sets()[0].distanceMeters(), 5000.0);
    EXPECT_EQ(run.sets()[0].durationSeconds(), 1440);
}

TEST_F(MixedWorkoutImportTest, Import_SeparatesTonnageFromTimeAndDistance)
{
    importMixed();

    const Workout workout = importedWorkout();

    EXPECT_DOUBLE_EQ(workout.totalWeight(), 1640.0);
    EXPECT_EQ(workout.totalDurationSeconds(), 1870);
    EXPECT_DOUBLE_EQ(workout.totalDistanceMeters(), 5000.0);
}

TEST_F(MixedWorkoutImportTest, Import_SurvivesTheDatabaseRoundtrip)
{
    importMixed();

    app.plannedWorkoutViewModel().loadAll();
    app.drain();

    const Workout reloaded = importedWorkout();

    ASSERT_EQ(reloaded.exercises().size(), 5u);
    EXPECT_EQ(reloaded.exercises()[3].kind(), ExerciseKind::Interval);
    EXPECT_EQ(reloaded.exercises()[3].setsToString(), "8x(20s/10s)");
    EXPECT_EQ(reloaded.exercises()[4].setsToString(), "5km@24min");
    EXPECT_EQ(reloaded.exercises()[1].setsToString(), "8xBW, 6xBW, 5xBW-10kg");
}

TEST_F(MixedWorkoutImportTest, CompletedMixedWorkout_ReachesHistoryWithItsMetrics)
{
    importMixed();

    auto& active = app.activeWorkoutViewModel();
    active.startWorkout(app.plannedWorkoutViewModel().workouts().first());

    for (auto* exercise : active.currentWorkout()->exercises())
    {
        for (int i = 0; i < exercise->sets().size(); ++i)
            active.completeCurrentSet();
    }

    active.endWorkout();
    app.drain();

    app.workoutHistoryViewModel().loadAllWorkouts();
    app.drain();

    const auto& history = app.workoutHistoryViewModel().workouts();
    ASSERT_FALSE(history.isEmpty());

    const Workout recorded = history.first()->toEntity();
    EXPECT_TRUE(recorded.isCompleted());
    EXPECT_DOUBLE_EQ(recorded.totalWeight(), 1640.0);
    EXPECT_DOUBLE_EQ(recorded.totalDistanceMeters(), 5000.0);
    EXPECT_EQ(recorded.exercises()[3].setsToString(), "8x(20s/10s)");
}

TEST_F(MixedWorkoutImportTest, CompactExport_FeedsTheGrammarBackToThePrompt)
{
    importMixed();

    const QJsonObject compact = WorkoutJson::workoutToJsonCompact(importedWorkout());
    const QJsonArray exercises = compact["exercises"].toArray();

    ASSERT_EQ(exercises.size(), 5);
    EXPECT_EQ(exercises[3].toObject()["sets"].toString(), "8x(20s/10s)");
    EXPECT_EQ(exercises[3].toObject()["kind"].toString(), "interval");
    EXPECT_EQ(exercises[4].toObject()["sets"].toString(), "5km@24min");
    EXPECT_EQ(exercises[0].toObject()["sets"].toString(), "10x60kg, 8x70kg, 6x80kg");
}

TEST_F(MixedWorkoutImportTest, BrokenSets_AreReportedInsteadOfSilentlyDropped)
{
    auto& planned = app.plannedWorkoutViewModel();
    QSignalSpy errorSpy(&planned, &PlannedWorkoutViewModel::errorOccurred);

    planned.importFromJson(TestData::BROKEN_SETS_JSON);
    planned.loadAll();
    app.drain();

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_TRUE(errorSpy.first().first().toString().contains("gibberish"));
}

TEST_F(MixedWorkoutImportTest, BrokenSets_StillImportTheReadableSets)
{
    auto& planned = app.plannedWorkoutViewModel();

    planned.importFromJson(TestData::BROKEN_SETS_JSON);
    planned.loadAll();
    app.drain();

    ASSERT_EQ(planned.workouts().size(), 1);
    const Workout workout = planned.workouts().first()->toEntity();
    ASSERT_EQ(workout.exercises().size(), 1u);
    EXPECT_EQ(workout.exercises()[0].sets().size(), 2u);
    EXPECT_EQ(workout.exercises()[0].setsToString(), "10x60kg, 6x80kg");
}

TEST_F(MixedWorkoutImportTest, ValidImport_ReportsNoErrors)
{
    auto& planned = app.plannedWorkoutViewModel();
    QSignalSpy errorSpy(&planned, &PlannedWorkoutViewModel::errorOccurred);

    importMixed();

    EXPECT_EQ(errorSpy.count(), 0);
}
