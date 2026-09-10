#include <QSignalSpy>
#include <gtest/gtest.h>

#include "application/workout/workoutservice.h"
#include "domain/workout/exercise.h"
#include "domain/workout/performedsets.h"
#include "domain/workout/workout.h"
#include "testapplication.h"
#include "ui/models/exercisemodel.h"
#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

class HistoryEditTest : public ::testing::Test
{
protected:
    WorkoutHistoryViewModel& history() { return m_app.workoutHistoryViewModel(); }

    static Set set(int repetitions, double weight, bool completed)
    {
        Set value(repetitions, weight);
        value.setCompleted(completed);
        return value;
    }

    void seedSession(const QString& name, std::initializer_list<Set> sets)
    {
        Exercise exercise
            = Exercise::createAdHoc(QStringLiteral("Back Squat"), ExerciseKind::Strength, 180);
        for (const Set& value : sets)
            exercise.addSet(value);

        Workout session(name, QDateTime(QDate(2025, 1, 3), QTime(18, 0, 0)));
        session.setStartedTime(QDateTime(QDate(2025, 1, 3), QTime(18, 0, 0)));
        session.setEndedTime(QDateTime(QDate(2025, 1, 3), QTime(19, 0, 0)));
        session.addExercise(exercise);

        m_app.workoutService()
            .importHistory(std::vector<Workout> { session })
            .warnOnError("seed the session");
        m_app.drain();

        history().loadAllWorkouts();
        m_app.drain();
    }

    WorkoutModel* session() { return history().workouts().first(); }

    SetModel* setAt(int index) { return session()->exercises().first()->sets().at(index); }

    Workout stored()
    {
        std::optional<Workout> found;
        m_app.workoutService()
            .findWorkout(session()->id())
            .then(&history(), [&found](std::optional<Workout> result) { found = result; })
            .warnOnError("reload the edited session");
        m_app.drain();

        return found.value_or(Workout());
    }

    static int tickedSets(const Workout& workout)
    {
        int ticked = 0;
        for (const Exercise& exercise : workout.exercises())
        {
            for (const Set& value : exercise.sets())
                ticked += value.completed() ? 1 : 0;
        }
        return ticked;
    }

    TestApplication m_app;
};

TEST_F(HistoryEditTest, ASkippedSetCanBeMarkedAsDone)
{
    seedSession(QStringLiteral("Summary check"), { set(5, 40.0, true), set(5, 40.0, false) });

    history().requestSetToggle(setAt(1));
    m_app.drain();

    EXPECT_TRUE(setAt(1)->completed());
    EXPECT_EQ(tickedSets(stored()), 2);
}

TEST_F(HistoryEditTest, ADoneSetCanBeMarkedAsSkipped)
{
    seedSession(QStringLiteral("Summary check"), { set(5, 40.0, true), set(5, 40.0, true) });

    history().requestSetToggle(setAt(0));
    m_app.drain();

    EXPECT_FALSE(setAt(0)->completed());
    EXPECT_EQ(tickedSets(stored()), 1);
}

TEST_F(HistoryEditTest, ATickedSessionTakesTheChangeWithoutAWarning)
{
    seedSession(QStringLiteral("Summary check"), { set(5, 40.0, true), set(5, 40.0, false) });

    QSignalSpy warnings(&history(), &WorkoutHistoryViewModel::amnestyWarningRaised);
    history().requestSetToggle(setAt(1));
    m_app.drain();

    EXPECT_EQ(warnings.count(), 0);
}

TEST_F(HistoryEditTest, TheFirstTickInAFlaglessSessionIsAnnouncedBeforeItHappens)
{
    seedSession(QStringLiteral("Base Strength"), { set(5, 40.0, false), set(5, 40.0, false) });

    QSignalSpy warnings(&history(), &WorkoutHistoryViewModel::amnestyWarningRaised);
    history().requestSetToggle(setAt(0));
    m_app.drain();

    ASSERT_EQ(warnings.count(), 1);
    EXPECT_TRUE(warnings.first().first().toString().contains(QStringLiteral("Base Strength")));
    EXPECT_FALSE(setAt(0)->completed());
    EXPECT_EQ(tickedSets(stored()), 0);
}

TEST_F(HistoryEditTest, CancellingTheWarningLeavesTheAmnestyInPlace)
{
    seedSession(QStringLiteral("Base Strength"), { set(5, 40.0, false), set(5, 40.0, false) });

    history().requestSetToggle(setAt(0));
    history().cancelPendingToggle();
    m_app.drain();

    EXPECT_FALSE(setAt(0)->completed());
    EXPECT_FALSE(completionFlagsAreMeaningful(session()->toEntity()));
    EXPECT_TRUE(setAt(1)->performed(session()->completionFlagsMeaningful()));
}

TEST_F(HistoryEditTest, ConfirmingTheWarningTicksTheSetAndEndsTheAmnesty)
{
    seedSession(QStringLiteral("Base Strength"), { set(5, 40.0, false), set(5, 40.0, false) });

    history().requestSetToggle(setAt(0));
    history().confirmPendingToggle();
    m_app.drain();

    EXPECT_TRUE(setAt(0)->completed());
    EXPECT_TRUE(session()->completionFlagsMeaningful());
    EXPECT_FALSE(setAt(1)->performed(session()->completionFlagsMeaningful()));
    EXPECT_EQ(tickedSets(stored()), 1);
}

TEST_F(HistoryEditTest, UntickingNeverRaisesTheWarning)
{
    seedSession(QStringLiteral("Summary check"), { set(5, 40.0, true), set(5, 40.0, false) });

    QSignalSpy warnings(&history(), &WorkoutHistoryViewModel::amnestyWarningRaised);
    history().requestSetToggle(setAt(0));
    m_app.drain();

    EXPECT_EQ(warnings.count(), 0);
    EXPECT_FALSE(setAt(0)->completed());
}

TEST_F(HistoryEditTest, TheEditedSessionKeepsItsPlaceInHistory)
{
    seedSession(QStringLiteral("Base Strength"), { set(5, 40.0, false), set(5, 40.0, false) });

    const int before = history().workouts().size();
    history().requestSetToggle(setAt(0));
    history().confirmPendingToggle();
    m_app.drain();

    EXPECT_EQ(history().workouts().size(), before);
    EXPECT_EQ(history().lastWorkout()->name(), QStringLiteral("Base Strength"));
}

TEST_F(HistoryEditTest, AStraySetWithoutAWorkoutIsIgnored)
{
    seedSession(QStringLiteral("Summary check"), { set(5, 40.0, true) });

    SetModel orphan(set(5, 40.0, false));
    QSignalSpy warnings(&history(), &WorkoutHistoryViewModel::amnestyWarningRaised);

    history().requestSetToggle(&orphan);
    history().requestSetToggle(nullptr);
    m_app.drain();

    EXPECT_EQ(warnings.count(), 0);
    EXPECT_FALSE(orphan.completed());
}
