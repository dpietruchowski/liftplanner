#include <gtest/gtest.h>

#include "application/workout/workoutservice.h"
#include "async/timeprovider.h"
#include "domain/exercisecatalog/exercisedefinition.h"
#include "domain/workout/generatedworkoutname.h"
#include "domain/workout/workout.h"
#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/models/exercisedefinitionmodel.h"
#include "ui/models/exercisemodel.h"
#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

class BlankWorkoutTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_app.setCurrentDateTime(QDateTime(QDate(2026, 9, 10), QTime(18, 30)));

        ExerciseDefinition squat(QStringLiteral("back-squat"), QStringLiteral("Back Squat"),
                                 ExerciseKind::Strength);
        squat.setDefaultMetric(SetMetric::Reps);
        squat.setDefaultLoadType(LoadType::External);
        squat.setDefaultRestSeconds(180);
        m_app.seedDefinition(squat);
        m_app.drain();

        catalog().load();
        m_app.drain();

        planned().loadAll();
        m_app.drain();
    }

    void TearDown() override
    {
        if (active().currentWorkout() != nullptr)
            active().discardWorkout();
        m_app.drain();
    }

    ActiveWorkoutViewModel& active() { return m_app.activeWorkoutViewModel(); }
    PlannedWorkoutViewModel& planned() { return m_app.plannedWorkoutViewModel(); }
    ExerciseCatalogViewModel& catalog() { return m_app.exerciseCatalogViewModel(); }
    WorkoutHistoryViewModel& history() { return m_app.workoutHistoryViewModel(); }

    void startBlankWorkout()
    {
        ASSERT_NE(planned().blankWorkout(), nullptr);
        active().startWorkout(planned().blankWorkout());
        m_app.drain();
    }

    void addSquat()
    {
        ExerciseDefinitionModel* model = nullptr;
        for (auto* candidate : catalog().exercises())
        {
            if (candidate->name() == QStringLiteral("Back Squat"))
                model = candidate;
        }

        ASSERT_NE(model, nullptr);
        active().addExercise(model);
        m_app.drain();
    }

    QList<WorkoutModel*> reloadPlanned()
    {
        planned().loadAll();
        m_app.drain();
        return planned().workouts();
    }

    QList<WorkoutModel*> reloadHistory()
    {
        history().loadAllWorkouts();
        m_app.drain();
        return history().workouts();
    }

    TestApplication m_app;
};

TEST_F(BlankWorkoutTest, TheOfferedSessionCarriesNoMomentAndHoldsNoExercise)
{
    ASSERT_NE(planned().blankWorkout(), nullptr);

    EXPECT_EQ(planned().blankWorkout()->name(), generatedPlanName());
    EXPECT_TRUE(planned().blankWorkout()->exercises().isEmpty());
}

TEST_F(BlankWorkoutTest, TheOfferedSessionFollowsTheDayWithoutRestartingTheApp)
{
    ASSERT_EQ(planned().blankWorkout()->plannedTime().date(), QDate(2026, 9, 10));

    m_app.advanceDay();
    planned().loadAll();
    m_app.drain();

    EXPECT_EQ(planned().blankWorkout()->plannedTime().date(), QDate(2026, 9, 11));
}

TEST_F(BlankWorkoutTest, StartingWithoutAPlanRunsAnEmptySessionStampedWithTheClock)
{
    startBlankWorkout();

    ASSERT_NE(active().currentWorkout(), nullptr);
    EXPECT_TRUE(active().isActive());
    EXPECT_EQ(active().exerciseCount(), 0);
    EXPECT_EQ(active().totalSetCount(), 0);
    EXPECT_EQ(active().currentWorkout()->name(), QStringLiteral("Freestyle · 18:30"));
}

TEST_F(BlankWorkoutTest, TwoSessionsOfOneDayAreToldApartInTheHistoryList)
{
    startBlankWorkout();
    addSquat();
    active().completeCurrentSet();
    active().endWorkout();
    m_app.drain();

    m_app.setCurrentDateTime(QDateTime(QDate(2026, 9, 10), QTime(7, 15)));
    planned().loadAll();
    m_app.drain();

    startBlankWorkout();
    addSquat();
    active().completeCurrentSet();
    active().endWorkout();
    m_app.drain();

    const auto stored = reloadHistory();
    ASSERT_EQ(stored.size(), 2);
    EXPECT_NE(stored.first()->name(), stored.last()->name());
    EXPECT_TRUE(stored.first()->name().contains(QStringLiteral(":")));
    EXPECT_TRUE(stored.last()->name().contains(QStringLiteral(":")));
}

TEST_F(BlankWorkoutTest, StartingWithoutAPlanLeavesTheOfferedSessionUntouched)
{
    startBlankWorkout();

    ASSERT_NE(planned().blankWorkout(), nullptr);
    EXPECT_NE(active().currentWorkout(), planned().blankWorkout());
    EXPECT_EQ(planned().blankWorkout()->statusString(), QStringLiteral("Planned"));
}

TEST_F(BlankWorkoutTest, AnExerciseAddedToAnEmptySessionArrivesWithASetToTickOff)
{
    startBlankWorkout();

    addSquat();

    ASSERT_EQ(active().exerciseCount(), 1);
    EXPECT_EQ(active().currentWorkout()->exercises().first()->name(), QStringLiteral("Back Squat"));
    EXPECT_EQ(active().totalSetCount(), 1);
    ASSERT_NE(active().currentSet(), nullptr);
    EXPECT_FALSE(active().currentSet()->completed());
}

TEST_F(BlankWorkoutTest, AnEmptySessionAnnouncesThatLeavingThrowsItAway)
{
    startBlankWorkout();

    const QString prompt = active().abandonPrompt();

    EXPECT_TRUE(prompt.contains(QStringLiteral("for good")));
    EXPECT_TRUE(prompt.contains(QStringLiteral("history")));
    EXPECT_TRUE(prompt.contains(QStringLiteral("planned list")));
}

TEST_F(BlankWorkoutTest, AbandoningAnEmptySessionLeavesNothingBehind)
{
    startBlankWorkout();

    active().discardWorkout();
    m_app.drain();

    EXPECT_EQ(active().currentWorkout(), nullptr);
    EXPECT_TRUE(reloadPlanned().isEmpty());
    EXPECT_TRUE(reloadHistory().isEmpty());
}

TEST_F(BlankWorkoutTest, AbandoningAnEmptySessionRemovesItsRowFromTheDatabase)
{
    startBlankWorkout();
    const int sessionId = active().currentWorkout()->id();
    ASSERT_NE(sessionId, -1);

    active().discardWorkout();
    m_app.drain();

    std::optional<Workout> found;
    m_app.workoutService()
        .findWorkout(sessionId)
        .then(&active(), [&found](std::optional<Workout> result) { found = result; })
        .warnOnError("look for the abandoned session");
    m_app.drain();

    EXPECT_FALSE(found.has_value());
}

TEST_F(BlankWorkoutTest, AbandoningASessionThatGotAnExerciseKeepsItOnThePlannedList)
{
    startBlankWorkout();
    addSquat();

    active().discardWorkout();
    m_app.drain();

    const auto planned = reloadPlanned();
    ASSERT_EQ(planned.size(), 1);
    EXPECT_EQ(planned.first()->name(), generatedPlanName());
    EXPECT_EQ(planned.first()->statusString(), QStringLiteral("Planned"));
    EXPECT_TRUE(reloadHistory().isEmpty());
}

TEST_F(BlankWorkoutTest, ReplacingAnEmptySessionLeavesNothingBehind)
{
    startBlankWorkout();

    planned().importFromJson(TestData::SINGLE_WORKOUT_JSON);
    planned().loadAll();
    m_app.drain();

    active().startWorkout(planned().workouts().first());
    m_app.drain();

    EXPECT_EQ(active().currentWorkout()->name(), QStringLiteral("Full Body"));
    EXPECT_TRUE(reloadHistory().isEmpty());

    for (const WorkoutModel* workout : reloadPlanned())
        EXPECT_NE(workout->name(), generatedPlanName());
}

TEST_F(BlankWorkoutTest, AFinishedFreeSessionReachesTheHistoryUnderItsClockTime)
{
    startBlankWorkout();
    addSquat();
    active().completeCurrentSet();

    active().endWorkout();
    m_app.drain();

    const auto stored = reloadHistory();
    ASSERT_EQ(stored.size(), 1);
    EXPECT_EQ(stored.first()->name(), QStringLiteral("Freestyle · 18:30"));
    EXPECT_EQ(stored.first()->statusString(), QStringLiteral("Ended"));
    EXPECT_TRUE(reloadPlanned().isEmpty());
}

TEST_F(BlankWorkoutTest, RepeatingAFreeSessionPlansItWithoutTheMomentOfTheOldOne)
{
    startBlankWorkout();
    addSquat();
    active().completeCurrentSet();
    active().endWorkout();
    m_app.drain();

    const auto stored = reloadHistory();
    ASSERT_EQ(stored.size(), 1);
    const QString sessionName = stored.first()->name();

    planned().repeatWorkout(stored.first());
    m_app.drain();

    const auto plans = reloadPlanned();
    ASSERT_EQ(plans.size(), 1);
    EXPECT_NE(plans.first()->name(), sessionName);
    EXPECT_EQ(plans.first()->name(), generatedPlanName());
}

TEST_F(BlankWorkoutTest, TheCopyOfAFreeSessionStampsItsOwnClockTimeWhenItStarts)
{
    startBlankWorkout();
    addSquat();
    active().completeCurrentSet();
    active().endWorkout();
    m_app.drain();

    planned().repeatWorkout(reloadHistory().first());
    m_app.drain();

    m_app.setCurrentDateTime(QDateTime(QDate(2026, 9, 12), QTime(9, 5)));

    active().startWorkout(reloadPlanned().first());
    m_app.drain();

    EXPECT_EQ(active().currentWorkout()->name(), QStringLiteral("Freestyle · 09:05"));
}

TEST_F(BlankWorkoutTest, RepeatingAWorkoutTheLifterNamedKeepsThatName)
{
    planned().importFromJson(TestData::SINGLE_WORKOUT_JSON);
    planned().loadAll();
    m_app.drain();

    active().startWorkout(planned().workouts().first());
    active().completeCurrentSet();
    active().endWorkout();
    m_app.drain();

    const auto stored = reloadHistory();
    ASSERT_EQ(stored.size(), 1);
    ASSERT_EQ(stored.first()->name(), QStringLiteral("Full Body"));

    planned().repeatWorkout(stored.first());
    m_app.drain();

    const auto plans = reloadPlanned();
    ASSERT_EQ(plans.size(), 1);
    EXPECT_EQ(plans.first()->name(), QStringLiteral("Full Body"));
}

TEST_F(BlankWorkoutTest, AStartedPlanTheLifterNamedKeepsItsNameThroughTheSession)
{
    planned().importFromJson(TestData::SINGLE_WORKOUT_JSON);
    planned().loadAll();
    m_app.drain();

    active().startWorkout(planned().workouts().first());
    m_app.drain();

    EXPECT_EQ(active().currentWorkout()->name(), QStringLiteral("Full Body"));
}
