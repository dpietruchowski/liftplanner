#include <gtest/gtest.h>

#include "application/workout/workoutservice.h"
#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"

class WorkoutRestoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        app.plannedWorkoutViewModel().importFromJson(TestData::THREE_WORKOUTS_JSON);
        app.plannedWorkoutViewModel().loadAll();
        app.drain();

        auto& vm = app.activeWorkoutViewModel();
        vm.startWorkout(app.plannedWorkoutViewModel().workouts().first());
        vm.completeCurrentSet();
        app.drain();
    }

    void TearDown() override
    {
        app.activeWorkoutViewModel().endWorkout();
        app.drain();
    }

    Workout storedWorkout()
    {
        std::optional<Workout> stored;
        app.workoutService()
            .findWorkout(app.activeWorkoutViewModel().currentWorkout()->id())
            .then(&app.activeWorkoutViewModel(),
                  [&stored](std::optional<Workout> found) { stored = found; })
            .warnOnError("read the stored workout");
        app.drain();
        return stored.value();
    }

    void relinkFirstExerciseInDatabase(int definitionId, const QString& name)
    {
        Workout stored = storedWorkout();
        stored.exercises()[0].setDefinitionId(definitionId);
        stored.exercises()[0].setName(name);
        app.workoutService().saveWorkout(stored).warnOnError("relink the first exercise");
        app.drain();
    }

    TestApplication app;
};

TEST_F(WorkoutRestoreTest, ARestartTakesTheExercisesFromTheDatabase)
{
    relinkFirstExerciseInDatabase(42, "Barbell Bench Press");

    ActiveWorkoutViewModel restored(&app.workoutService());
    app.drain();

    ASSERT_NE(restored.currentWorkout(), nullptr);
    EXPECT_TRUE(restored.isActive());
    EXPECT_EQ(restored.currentWorkout()->id(), app.activeWorkoutViewModel().currentWorkout()->id());
    EXPECT_EQ(restored.currentWorkout()->exercises().first()->name(), "Barbell Bench Press");
    EXPECT_EQ(restored.currentWorkout()->exercises().first()->toEntity().definitionId(), 42);
}

TEST_F(WorkoutRestoreTest, SavingFromTheActiveWorkoutKeepsTheCatalogLink)
{
    relinkFirstExerciseInDatabase(42, "Barbell Bench Press");

    ActiveWorkoutViewModel restored(&app.workoutService());
    app.drain();
    ASSERT_NE(restored.currentWorkout(), nullptr);

    restored.completeCurrentSet();
    app.drain();

    const Workout stored = storedWorkout();
    EXPECT_EQ(stored.exercises()[0].definitionId(), 42);
    EXPECT_EQ(stored.exercises()[0].name(), "Barbell Bench Press");
    EXPECT_EQ(stored.exercises()[0].sets().size(),
              restored.currentWorkout()->exercises().first()->sets().size());
}

TEST_F(WorkoutRestoreTest, ARestartKeepsTheCompletionStateFromTheCache)
{
    Workout stored = storedWorkout();
    for (Exercise& exercise : stored.exercises())
    {
        for (Set& set : exercise.sets())
            set.setCompleted(false);
    }
    app.workoutService().saveWorkout(stored).warnOnError("clear the completed sets");
    app.drain();

    ActiveWorkoutViewModel restored(&app.workoutService());
    app.drain();

    ASSERT_NE(restored.currentWorkout(), nullptr);
    const auto sets = restored.currentWorkout()->exercises().first()->sets();
    EXPECT_TRUE(sets.first()->completed());
    EXPECT_FALSE(sets.last()->completed());
    ASSERT_NE(restored.currentSet(), nullptr);
    EXPECT_FALSE(restored.currentSet()->completed());
}

TEST_F(WorkoutRestoreTest, ARestartFallsBackToTheCacheWhenTheDatabaseLostTheWorkout)
{
    app.workoutService()
        .deleteWorkout(app.activeWorkoutViewModel().currentWorkout()->id())
        .warnOnError("delete the workout behind the active one");
    app.drain();

    ActiveWorkoutViewModel restored(&app.workoutService());
    app.drain();

    ASSERT_NE(restored.currentWorkout(), nullptr);
    EXPECT_EQ(restored.currentWorkout()->name(), "Push Day");
    EXPECT_TRUE(restored.currentWorkout()->exercises().first()->sets().first()->completed());
}
