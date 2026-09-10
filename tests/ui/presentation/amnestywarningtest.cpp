#include "ui/presentation/workouttext.h"

#include <gtest/gtest.h>

TEST(AmnestyWarningTest, TheWarningNamesTheSessionAndTheSetsItWouldSkip)
{
    EXPECT_EQ(WorkoutText::amnestyLossWarning(QStringLiteral("Base Strength"), 22),
              QStringLiteral("\"Base Strength\" has no ticked sets, so the whole session counts "
                             "as done. Ticking this one marks the other 21 sets as skipped."));
}

TEST(AmnestyWarningTest, TheRemainderIsCountedInTheSingularWhenOnlyOneSetIsLeft)
{
    const QString warning = WorkoutText::amnestyLossWarning(QStringLiteral("Summary check"), 2);

    EXPECT_TRUE(warning.contains(QStringLiteral("the other 1 set as skipped")));
}

TEST(AmnestyWarningTest, ASessionOfASingleSetLeavesNothingBehind)
{
    const QString warning = WorkoutText::amnestyLossWarning(QStringLiteral("Smoke"), 1);

    EXPECT_TRUE(warning.contains(QStringLiteral("the other 0 sets as skipped")));
}

TEST(AmnestyWarningTest, AnUnnamedSessionIsStillDescribed)
{
    const QString warning = WorkoutText::amnestyLossWarning(QString(), 3);

    EXPECT_TRUE(warning.startsWith(QStringLiteral("This session has no ticked sets")));
}
