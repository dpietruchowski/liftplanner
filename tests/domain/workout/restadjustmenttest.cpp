#include "domain/workout/restadjustment.h"
#include <gtest/gtest.h>

TEST(RestAdjustmentTest, OneStepUpAddsAQuarterOfAMinute)
{
    EXPECT_EQ(RestAdjustment::adjusted(120, 1), 135);
}

TEST(RestAdjustmentTest, OneStepDownRemovesAQuarterOfAMinute)
{
    EXPECT_EQ(RestAdjustment::adjusted(120, -1), 105);
}

TEST(RestAdjustmentTest, SeveralStepsMoveByTheirSum)
{
    EXPECT_EQ(RestAdjustment::adjusted(120, 4), 180);
    EXPECT_EQ(RestAdjustment::adjusted(120, -4), 60);
}

TEST(RestAdjustmentTest, AnImportedOffGridValueSnapsToTheGrid)
{
    EXPECT_EQ(RestAdjustment::adjusted(100, 1), 105);
    EXPECT_EQ(RestAdjustment::adjusted(100, -1), 90);
}

TEST(RestAdjustmentTest, RestNeverDropsBelowThePracticalMinimum)
{
    EXPECT_EQ(RestAdjustment::adjusted(15, -1), 15);
    EXPECT_EQ(RestAdjustment::adjusted(20, -3), 15);
    EXPECT_EQ(RestAdjustment::adjusted(0, -1), 15);
}

TEST(RestAdjustmentTest, RestNeverGrowsBeyondTenMinutes)
{
    EXPECT_EQ(RestAdjustment::adjusted(600, 1), 600);
    EXPECT_EQ(RestAdjustment::adjusted(590, 5), 600);
}

TEST(RestAdjustmentTest, AValueOutsideTheRangeIsPulledBackInWithoutSteps)
{
    EXPECT_EQ(RestAdjustment::adjusted(5, 0), 15);
    EXPECT_EQ(RestAdjustment::adjusted(1200, 0), 600);
}

TEST(RestAdjustmentTest, SteppingUpThenDownReturnsToTheSameValue)
{
    EXPECT_EQ(RestAdjustment::adjusted(RestAdjustment::adjusted(90, 1), -1), 90);
}
