#include <gtest/gtest.h>

#include "domain/workout/blanksession.h"

namespace
{

const QDateTime septemberEvening { QDate(2026, 9, 10), QTime(18, 30) };

}

TEST(BlankSessionTest, ASessionStartedWithoutAPlanCarriesTheDayInItsName)
{
    const Workout session = blankSession(septemberEvening);

    EXPECT_TRUE(session.name().contains(QStringLiteral("10 Sep")));
}

TEST(BlankSessionTest, TheNameIsNeverTheEmptyDashTheDrumFallsBackTo)
{
    const Workout session = blankSession(septemberEvening);

    EXPECT_FALSE(session.name().trimmed().isEmpty());
    EXPECT_NE(session.name(), QStringLiteral("---"));
}

TEST(BlankSessionTest, TwoSessionsOnDifferentDaysAreToldApartByTheirNames)
{
    const Workout first = blankSession(septemberEvening);
    const Workout second = blankSession(septemberEvening.addDays(1));

    EXPECT_NE(first.name(), second.name());
}

TEST(BlankSessionTest, TwoSessionsOnTheSameDayShareTheirName)
{
    const Workout morning = blankSession(QDateTime(QDate(2026, 9, 10), QTime(7, 0)));
    const Workout evening = blankSession(septemberEvening);

    EXPECT_EQ(morning.name(), evening.name());
}

TEST(BlankSessionTest, ASessionStartedWithoutAPlanHoldsNoExercise)
{
    const Workout session = blankSession(septemberEvening);

    EXPECT_TRUE(session.isEmpty());
    EXPECT_EQ(session.totalSets(), 0);
}

TEST(BlankSessionTest, ASessionStartedWithoutAPlanIsDatedSoTheDrumCanShowItsDay)
{
    const Workout session = blankSession(septemberEvening);

    EXPECT_EQ(session.plannedTime(), septemberEvening);
    EXPECT_EQ(session.createdTime(), septemberEvening);
}

TEST(BlankSessionTest, ANameWithoutADayStillSaysWhatKindOfSessionItIs)
{
    EXPECT_FALSE(blankSessionName(QDate()).trimmed().isEmpty());
}
