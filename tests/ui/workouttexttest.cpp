#include "ui/presentation/workouttext.h"
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

TEST_F(WorkoutTextTest, FormatRest_ReadsAsMinutesAndPaddedSeconds)
{
    EXPECT_EQ(WorkoutText::formatRest(120), "2:00");
    EXPECT_EQ(WorkoutText::formatRest(90), "1:30");
    EXPECT_EQ(WorkoutText::formatRest(45), "0:45");
    EXPECT_EQ(WorkoutText::formatRest(605), "10:05");
}

TEST_F(WorkoutTextTest, FormatRest_TreatsMissingRestAsZero)
{
    EXPECT_EQ(WorkoutText::formatRest(0), "0:00");
    EXPECT_EQ(WorkoutText::formatRest(-30), "0:00");
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

TEST_F(WorkoutTextTest, FormatVolume_StaysInKilogramsAndRoundsToWholeOnes)
{
    EXPECT_EQ(WorkoutText::formatVolume(0.0), "0 kg");
    EXPECT_EQ(WorkoutText::formatVolume(900.0), "900 kg");
    EXPECT_EQ(WorkoutText::formatVolume(987.5), "988 kg");
}

TEST_F(WorkoutTextTest, FormatVolume_GroupsThousandsSoBigNumbersStayReadable)
{
    EXPECT_EQ(WorkoutText::formatVolume(1000.0), "1 000 kg");
    EXPECT_EQ(WorkoutText::formatVolume(12400.0), "12 400 kg");
    EXPECT_EQ(WorkoutText::formatVolume(203450.0), "203 450 kg");
    EXPECT_EQ(WorkoutText::formatVolume(1234567.0), "1 234 567 kg");
}

TEST_F(WorkoutTextTest, FormatVolume_NeverReportsNegativeWork)
{
    EXPECT_EQ(WorkoutText::formatVolume(-50.0), "0 kg");
}
