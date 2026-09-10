#include <QJsonDocument>
#include <gtest/gtest.h>

#include "application/workout/workoutservice.h"
#include "domain/workout/exercise.h"
#include "domain/workout/performedsets.h"
#include "domain/workout/workout.h"
#include "testapplication.h"
#include "ui/models/exercisemodel.h"
#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

class HistoryRoundTripTest : public ::testing::Test
{
protected:
    WorkoutHistoryViewModel& history() { return m_app.workoutHistoryViewModel(); }
    PlannedWorkoutViewModel& planned() { return m_app.plannedWorkoutViewModel(); }

    static Set set(int repetitions, double weight, bool completed)
    {
        Set value(repetitions, weight);
        value.setCompleted(completed);
        return value;
    }

    void seedSession(const QString& name, std::initializer_list<Set> sets, const QDate& date)
    {
        Exercise exercise
            = Exercise::createAdHoc(QStringLiteral("Back Squat"), ExerciseKind::Strength, 180);
        for (const Set& value : sets)
            exercise.addSet(value);

        Workout session(name, QDateTime(date, QTime(18, 0, 0)));
        session.setStartedTime(QDateTime(date, QTime(18, 0, 0)));
        session.setEndedTime(QDateTime(date, QTime(19, 0, 0)));
        session.addExercise(exercise);

        m_app.workoutService()
            .importHistory(std::vector<Workout> { session })
            .warnOnError("seed the session");
        m_app.drain();
    }

    QString exportedHistory()
    {
        history().loadAllWorkouts();
        m_app.drain();

        const QJsonDocument doc(history().recentWorkoutsToJson(50));
        return QString::fromUtf8(doc.toJson());
    }

    void wipeAndReimport(const QString& payload)
    {
        for (WorkoutModel* workout : history().workouts())
        {
            m_app.workoutService()
                .deleteWorkout(workout->id())
                .warnOnError("clear the history before the reimport");
        }
        m_app.drain();

        history().importFromJson(payload);
        m_app.drain();
        history().loadAllWorkouts();
        m_app.drain();
    }

    Workout restored(const QString& name)
    {
        for (WorkoutModel* workout : history().workouts())
        {
            if (workout->name() == name)
                return workout->toEntity();
        }
        return Workout();
    }

    TestApplication m_app;
};

TEST_F(HistoryRoundTripTest, APartlyTickedSessionKeepsExactlyItsTicks)
{
    seedSession(QStringLiteral("Summary check"), { set(5, 40.0, true), set(5, 40.0, false) },
                QDate(2025, 1, 3));

    wipeAndReimport(exportedHistory());

    const Workout session = restored(QStringLiteral("Summary check"));
    ASSERT_EQ(session.exercises().size(), 1u);
    ASSERT_EQ(session.exercises()[0].sets().size(), 2u);
    EXPECT_TRUE(session.exercises()[0].sets()[0].completed());
    EXPECT_FALSE(session.exercises()[0].sets()[1].completed());
    EXPECT_TRUE(completionFlagsAreMeaningful(session));
}

TEST_F(HistoryRoundTripTest, ASessionWithoutASingleTickStaysUnderTheAmnesty)
{
    seedSession(QStringLiteral("Base Strength"), { set(5, 40.0, false), set(5, 40.0, false) },
                QDate(2025, 1, 4));

    wipeAndReimport(exportedHistory());

    const Workout session = restored(QStringLiteral("Base Strength"));
    ASSERT_EQ(session.exercises().size(), 1u);
    EXPECT_FALSE(completionFlagsAreMeaningful(session));
    EXPECT_TRUE(wasPerformed(session.exercises()[0], completionFlagsAreMeaningful(session)));
}

TEST_F(HistoryRoundTripTest, TheTickCountSurvivesTheRoundTrip)
{
    seedSession(QStringLiteral("Summary check"),
                { set(5, 40.0, true), set(5, 40.0, false), set(5, 40.0, false) },
                QDate(2025, 1, 5));

    wipeAndReimport(exportedHistory());

    const Workout session = restored(QStringLiteral("Summary check"));
    ASSERT_EQ(session.exercises().size(), 1u);

    int ticked = 0;
    for (const Set& value : session.exercises()[0].sets())
        ticked += value.completed() ? 1 : 0;

    EXPECT_EQ(session.exercises()[0].sets().size(), 3u);
    EXPECT_EQ(ticked, 1);
}

TEST_F(HistoryRoundTripTest, APlanImportedFromAMarkedPayloadStillArrivesUndone)
{
    const QString plan = QStringLiteral(
        "{\"user_profile\": null, \"workouts\": [{\"name\": \"Next Week\","
        " \"exercises\": [{\"name\": \"Back Squat\", \"sets\": \"5x40kg!, 5x40kg\","
        " \"rest_seconds\": 180}]}]}");

    planned().importFromJson(plan);
    planned().loadAll();
    m_app.drain();

    ASSERT_EQ(planned().workouts().size(), 1);
    WorkoutModel* workout = planned().workouts().first();
    EXPECT_EQ(workout->status(), WorkoutStatus::Planned);

    ASSERT_EQ(workout->exercises().size(), 1);
    const QList<SetModel*> sets = workout->exercises().first()->sets();
    ASSERT_EQ(sets.size(), 2);
    for (const SetModel* value : sets)
        EXPECT_FALSE(value->completed());
}

TEST_F(HistoryRoundTripTest, APlainAiPlanIsUnaffected)
{
    const QString plan
        = QStringLiteral("{\"user_profile\": null, \"workouts\": [{\"name\": \"Next Week\","
                         " \"exercises\": [{\"name\": \"Back Squat\", \"sets\": \"5x40kg, 5x40kg\","
                         " \"rest_seconds\": 180}]}]}");

    planned().importFromJson(plan);
    planned().loadAll();
    m_app.drain();

    ASSERT_EQ(planned().workouts().size(), 1);
    EXPECT_EQ(planned().workouts().first()->exercises().first()->sets().size(), 2);
}
