#include "ui/presentation/workouttext.h"

#include <gtest/gtest.h>

namespace
{
const QDate today(2026, 9, 10);
}

TEST(PlannedDayTest, TodayIsNamedAsToday)
{
    EXPECT_EQ(WorkoutText::plannedDayLabel(today, today),
              QStringLiteral("Today · Thu, 10 Sep 2026"));
}

TEST(PlannedDayTest, TheNextDayIsNamedAsTomorrow)
{
    EXPECT_EQ(WorkoutText::plannedDayLabel(today.addDays(1), today),
              QStringLiteral("Tomorrow · Fri, 11 Sep 2026"));
}

TEST(PlannedDayTest, AFurtherDayIsShownByItsDate)
{
    EXPECT_EQ(WorkoutText::plannedDayLabel(today.addDays(4), today),
              QStringLiteral("Mon, 14 Sep 2026"));
}

TEST(PlannedDayTest, APastDayIsShownByItsDateWithoutAWord)
{
    EXPECT_EQ(WorkoutText::plannedDayLabel(today.addDays(-1), today),
              QStringLiteral("Wed, 9 Sep 2026"));
}

TEST(PlannedDayTest, AMissingDayIsSaidOutLoud)
{
    EXPECT_EQ(WorkoutText::plannedDayLabel(QDate(), today), QStringLiteral("No day set"));
}
