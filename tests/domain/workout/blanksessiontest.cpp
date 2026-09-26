#include <gtest/gtest.h>

#include "domain/workout/blanksession.h"
#include "domain/workout/generatedworkoutname.h"
#include "domain/workout/workoutrepeat.h"

namespace
{

const QDateTime septemberEvening { QDate(2026, 9, 10), QTime(18, 30) };

}

TEST(BlankSessionTest, TheOfferedSessionIsNamedWithoutAMomentBecauseItHasNotHappenedYet)
{
    const Workout session = blankSession(septemberEvening);

    EXPECT_EQ(session.name(), generatedPlanName());
    EXPECT_FALSE(session.name().trimmed().isEmpty());
}

TEST(BlankSessionTest, TheOfferedSessionCarriesTheDayItIsOfferedFor)
{
    const Workout session = blankSession(septemberEvening);

    EXPECT_EQ(session.plannedTime(), septemberEvening);
    EXPECT_EQ(session.createdTime(), septemberEvening);
}

TEST(BlankSessionTest, TheOfferedSessionHoldsNoExerciseAndNamesItselfAutomatically)
{
    const Workout session = blankSession(septemberEvening);

    EXPECT_TRUE(session.isEmpty());
    EXPECT_EQ(session.totalSets(), 0);
    EXPECT_TRUE(session.hasGeneratedName());
}

TEST(GeneratedWorkoutNameTest, ASessionNameCarriesTheClockTimeAndNotTheDate)
{
    const QString name = generatedSessionName(septemberEvening);

    EXPECT_TRUE(name.contains(QStringLiteral("18:30")));
    EXPECT_FALSE(name.contains(QStringLiteral("Sep")));
    EXPECT_FALSE(name.contains(QStringLiteral("10")));
}

TEST(GeneratedWorkoutNameTest, TwoSessionsOfOneDayAreToldApartByTheirNames)
{
    const QString morning = generatedSessionName(QDateTime(QDate(2026, 9, 10), QTime(7, 15)));
    const QString evening = generatedSessionName(septemberEvening);

    EXPECT_NE(morning, evening);
}

TEST(GeneratedWorkoutNameTest, APlanNameCarriesNoMomentAtAll)
{
    const QString planned = generatedPlanName();

    EXPECT_FALSE(planned.contains(QLatin1Char(':')));
    EXPECT_FALSE(planned.contains(QStringLiteral("Sep")));
    EXPECT_NE(planned, generatedSessionName(septemberEvening));
}

TEST(GeneratedWorkoutNameTest, AMomentThatIsNotSetFallsBackToThePlanName)
{
    EXPECT_EQ(generatedSessionName(QDateTime()), generatedPlanName());
}

TEST(BlankSessionTest, StartingTheSessionStampsTheClockTimeIntoItsName)
{
    Workout session = blankSession(septemberEvening);

    session.start();

    EXPECT_EQ(session.name(), generatedSessionName(session.startedTime()));
    EXPECT_NE(session.name(), generatedPlanName());
}

TEST(BlankSessionTest, StartingAWorkoutTheLifterNamedLeavesItsNameAlone)
{
    Workout push(QStringLiteral("Push A"), septemberEvening);

    push.start();

    EXPECT_EQ(push.name(), QStringLiteral("Push A"));
}

TEST(BlankSessionTest, AbandonedSessionDropsTheClockTimeWhenItBecomesAPlanAgain)
{
    Workout session = blankSession(septemberEvening);
    session.start();

    session.returnToPlan();

    EXPECT_EQ(session.name(), generatedPlanName());
    EXPECT_EQ(session.status(), WorkoutStatus::Planned);
}

TEST(BlankSessionTest, AWorkoutTheLifterNamedKeepsItsNameWhenItReturnsToThePlan)
{
    Workout push(QStringLiteral("Push A"), septemberEvening);
    push.start();

    push.returnToPlan();

    EXPECT_EQ(push.name(), QStringLiteral("Push A"));
    EXPECT_EQ(push.status(), WorkoutStatus::Planned);
}

TEST(BlankSessionTest, RepeatingAFreeSessionPlansItWithoutTheMomentOfTheOldOne)
{
    Workout session = blankSession(septemberEvening);
    session.start();
    const QString sessionName = session.name();

    const Workout repeated = repeatOf(session, QDateTime(QDate(2026, 9, 12), QTime(9, 0)));

    EXPECT_NE(repeated.name(), sessionName);
    EXPECT_EQ(repeated.name(), generatedPlanName());
    EXPECT_TRUE(repeated.hasGeneratedName());
}

TEST(BlankSessionTest, RepeatingAWorkoutTheLifterNamedKeepsThatName)
{
    Workout push(QStringLiteral("Push A"), septemberEvening);
    push.start();

    const Workout repeated = repeatOf(push, QDateTime(QDate(2026, 9, 12), QTime(9, 0)));

    EXPECT_EQ(repeated.name(), QStringLiteral("Push A"));
    EXPECT_FALSE(repeated.hasGeneratedName());
}

TEST(BlankSessionTest, TheCopyOfAFreeSessionStampsItsOwnClockTimeWhenItStarts)
{
    Workout session = blankSession(septemberEvening);
    session.start();

    Workout repeated = repeatOf(session, QDateTime(QDate(2026, 9, 12), QTime(9, 0)));
    repeated.start();

    EXPECT_EQ(repeated.name(), generatedSessionName(repeated.startedTime()));
}
