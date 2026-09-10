#include <gtest/gtest.h>

#include "application/workout/workoutservice.h"
#include "domain/workout/exercise.h"
#include "domain/workout/workout.h"
#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/models/exercisemodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"

class ExerciseHistoryPanelTest : public ::testing::Test
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

    static Set tickedSet(int repetitions, double weight)
    {
        Set set(repetitions, weight);
        set.setCompleted(true);
        return set;
    }

    static Exercise benchPress(std::initializer_list<Set> sets)
    {
        Exercise exercise
            = Exercise::createAdHoc(QStringLiteral("Bench Press"), ExerciseKind::Strength, 180);
        for (const Set& set : sets)
            exercise.addSet(set);
        return exercise;
    }

    void endSessionWith(const Exercise& exercise, const QDate& date)
    {
        Workout past(QStringLiteral("Base Strength"), QDateTime(date, QTime(18, 0, 0)));
        past.setStartedTime(QDateTime(date, QTime(18, 0, 0)));
        past.setEndedTime(QDateTime(date, QTime(19, 0, 0)));
        past.addExercise(exercise);

        app.workoutService()
            .importHistory(std::vector<Workout> { past })
            .warnOnError("seed a past session");
        app.drain();
    }

    QVariantList sessionsOfFirstExercise()
    {
        auto& vm = app.activeWorkoutViewModel();
        vm.loadExerciseSessions(vm.currentWorkout()->exercises().first());
        app.drain();
        return vm.exerciseSessions();
    }

    void TearDown() override
    {
        app.activeWorkoutViewModel().endWorkout();
        app.drain();
    }

    TestApplication app;
};

TEST_F(ExerciseHistoryPanelTest, ListsTheLastSessionsNewestFirst)
{
    endSessionWith(benchPress({ tickedSet(5, 80.0) }), QDate(2024, 12, 2));
    endSessionWith(benchPress({ tickedSet(5, 85.0) }), QDate(2024, 12, 9));
    endSessionWith(benchPress({ tickedSet(5, 90.0) }), QDate(2024, 12, 16));

    importAndStartFirstWorkout();

    const QVariantList sessions = sessionsOfFirstExercise();

    ASSERT_EQ(sessions.size(), 3);
    EXPECT_TRUE(sessions[0].toMap()["summary"].toString().contains(QStringLiteral("90")));
    EXPECT_TRUE(sessions[1].toMap()["summary"].toString().contains(QStringLiteral("85")));
    EXPECT_TRUE(sessions[2].toMap()["summary"].toString().contains(QStringLiteral("80")));
    EXPECT_FALSE(sessions[0].toMap()["date"].toString().isEmpty());
}

TEST_F(ExerciseHistoryPanelTest, ShowsAtMostFiveSessions)
{
    for (int day = 1; day <= 8; ++day)
        endSessionWith(benchPress({ tickedSet(5, 60.0 + day) }), QDate(2024, 12, day));

    importAndStartFirstWorkout();

    EXPECT_EQ(sessionsOfFirstExercise().size(), 5);
}

TEST_F(ExerciseHistoryPanelTest, CountsASessionImportedWithoutASingleTick)
{
    endSessionWith(benchPress({ Set(8, 40.0) }), QDate(2024, 6, 14));

    importAndStartFirstWorkout();

    const QVariantList sessions = sessionsOfFirstExercise();

    ASSERT_EQ(sessions.size(), 1);
    EXPECT_TRUE(sessions[0].toMap()["summary"].toString().contains(QStringLiteral("40")))
        << sessions[0].toMap()["summary"].toString().toStdString();
}

TEST_F(ExerciseHistoryPanelTest, StaysEmptyForAnExerciseDoneForTheFirstTime)
{
    importAndStartFirstWorkout();

    EXPECT_TRUE(sessionsOfFirstExercise().isEmpty());
}
