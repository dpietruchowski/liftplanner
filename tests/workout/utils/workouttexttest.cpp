#include "utils/workouttext.h"
#include <gtest/gtest.h>

class WorkoutTextTest : public ::testing::Test
{
};

TEST_F(WorkoutTextTest, FormatDuration_DropsToSecondsBelowAMinute)
{
    EXPECT_EQ(WorkoutText::formatDuration(0), "0s");
    EXPECT_EQ(WorkoutText::formatDuration(45), "45s");
}

TEST_F(WorkoutTextTest, FormatDuration_UsesMinutesThenHours)
{
    EXPECT_EQ(WorkoutText::formatDuration(60), "1m");
    EXPECT_EQ(WorkoutText::formatDuration(1560), "26m");
    EXPECT_EQ(WorkoutText::formatDuration(3600), "1h 00m");
    EXPECT_EQ(WorkoutText::formatDuration(4500), "1h 15m");
}

TEST_F(WorkoutTextTest, FormatDistance_UsesMetresBelowAKilometre)
{
    EXPECT_EQ(WorkoutText::formatDistance(0.0), "0 m");
    EXPECT_EQ(WorkoutText::formatDistance(400.0), "400 m");
    EXPECT_EQ(WorkoutText::formatDistance(999.4), "999 m");
}

TEST_F(WorkoutTextTest, FormatDistance_SwitchesToKilometresAndTrimsTheZeroDecimal)
{
    EXPECT_EQ(WorkoutText::formatDistance(1000.0), "1 km");
    EXPECT_EQ(WorkoutText::formatDistance(5000.0), "5 km");
    EXPECT_EQ(WorkoutText::formatDistance(12400.0), "12.4 km");
}
