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

    Exercise benchPress(std::initializer_list<Set> sets)
    {
        Exercise exercise
            = Exercise::createAdHoc(QStringLiteral("Bench Press"), ExerciseKind::Strength, 180);
        for (const Set& set : sets)
            exercise.addSet(set);
        return exercise;
    }

    void endSessionWith(const std::vector<Exercise>& exercises, const QDate& date)
    {
        Workout past(QStringLiteral("Base Strength"), QDateTime(date, QTime(18, 0, 0)));
        past.setStartedTime(QDateTime(date, QTime(18, 0, 0)));
        past.setEndedTime(QDateTime(date, QTime(19, 0, 0)));
        for (const Exercise& exercise : exercises)
            past.addExercise(exercise);

        app.workoutService()
            .importHistory(std::vector<Workout> { past })
            .warnOnError("seed the previous session");
        app.drain();
    }

    Exercise overheadPress(std::initializer_list<Set> sets)
    {
        Exercise exercise
            = Exercise::createAdHoc(QStringLiteral("Overhead Press"), ExerciseKind::Strength, 90);
        for (const Set& set : sets)
            exercise.addSet(set);
        return exercise;
    }

    static Set tickedSet(int repetitions, double weight)
    {
        Set set(repetitions, weight);
        set.setCompleted(true);
        return set;
    }

    QString previousSummaryOfFirstExercise()
    {
        return app.activeWorkoutViewModel()
            .currentWorkout()
            ->exercises()
            .first()
            ->previousSummary();
    }

    QStringList previousSetTextsOfFirstExercise()
    {
        return app.activeWorkoutViewModel()
            .currentWorkout()
            ->exercises()
            .first()
            ->previousSetTexts();
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

TEST_F(PreviousPerformanceTest, AFinishedSessionWithoutASingleTickIsStillLastTime)
{
    const QDate session(2024, 12, 20);
    endSessionWith({ benchPress({ Set(5, 20.0), Set(5, 30.0), Set(5, 45.0) }) }, session);

    importAndStartFirstWorkout();

    const QString summary = previousSummaryOfFirstExercise();
    EXPECT_FALSE(summary.isEmpty());
    EXPECT_TRUE(summary.contains(QStringLiteral("45"))) << summary.toStdString();
    EXPECT_TRUE(summary.contains(QStringLiteral("20"))) << summary.toStdString();
    EXPECT_EQ(
        app.activeWorkoutViewModel().currentWorkout()->exercises().first()->previousDate().date(),
        session);
}

TEST_F(PreviousPerformanceTest, AnUntickedExerciseInAPartlyTickedSessionStaysSilent)
{
    endSessionWith({ benchPress({ Set(5, 45.0) }), overheadPress({ tickedSet(8, 35.0) }) },
                   QDate(2024, 12, 20));

    importAndStartFirstWorkout();

    EXPECT_TRUE(previousSummaryOfFirstExercise().isEmpty());
}

TEST_F(PreviousPerformanceTest, AnExerciseWithoutAnySetsIsNotAPreviousPerformance)
{
    endSessionWith({ benchPress({}) }, QDate(2024, 12, 20));

    importAndStartFirstWorkout();

    EXPECT_TRUE(previousSummaryOfFirstExercise().isEmpty());
}

TEST_F(PreviousPerformanceTest, EverySetLearnsWhatTheSamePositionCarriedLastTime)
{
    endSessionWith({ benchPress({ tickedSet(10, 55.0), tickedSet(8, 65.0), tickedSet(6, 75.0) }) },
                   QDate(2024, 12, 20));

    importAndStartFirstWorkout();

    const QStringList hints = previousSetTextsOfFirstExercise();

    ASSERT_EQ(hints.size(), 3);
    EXPECT_EQ(hints[0], QStringLiteral("10x55kg"));
    EXPECT_EQ(hints[1], QStringLiteral("8x65kg"));
    EXPECT_EQ(hints[2], QStringLiteral("6x75kg"));
}

TEST_F(PreviousPerformanceTest, ASetTheLastSessionNeverReachedStaysWithoutAHint)
{
    endSessionWith({ benchPress({ tickedSet(10, 55.0) }) }, QDate(2024, 12, 20));

    importAndStartFirstWorkout();

    const QStringList hints = previousSetTextsOfFirstExercise();

    ASSERT_EQ(hints.size(), 3);
    EXPECT_EQ(hints[0], QStringLiteral("10x55kg"));
    EXPECT_TRUE(hints[1].isEmpty());
    EXPECT_TRUE(hints[2].isEmpty());
}

TEST_F(PreviousPerformanceTest, ASessionWithoutASingleTickStillFeedsEverySetHint)
{
    endSessionWith({ benchPress({ Set(8, 40.0), Set(8, 42.5) }) }, QDate(2024, 6, 14));

    importAndStartFirstWorkout();

    const QStringList hints = previousSetTextsOfFirstExercise();

    ASSERT_EQ(hints.size(), 3);
    EXPECT_EQ(hints[0], QStringLiteral("8x40kg"));
    EXPECT_EQ(hints[1], QStringLiteral("8x42.5kg"));
    EXPECT_TRUE(hints[2].isEmpty());
}

TEST_F(PreviousPerformanceTest, AFirstTimeExerciseHasNoSetHintsAtAll)
{
    importAndStartFirstWorkout();

    EXPECT_TRUE(previousSetTextsOfFirstExercise().isEmpty());
}
