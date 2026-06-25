#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

class TimeTravelTest : public ::testing::Test
{
protected:
    void recordWorkout()
    {
        app.plannedWorkoutViewModel().importFromJson(TestData::SINGLE_WORKOUT_JSON);
        app.plannedWorkoutViewModel().loadAll();
        app.drain();

        auto& active = app.activeWorkoutViewModel();
        active.startWorkout(app.plannedWorkoutViewModel().workouts().first());
        active.endWorkout();

        app.workoutHistoryViewModel().loadAllWorkouts();
        app.drain();
    }

    int activeDays()
    {
        const QVariantList activity = app.workoutHistoryViewModel().weekActivity();
        int count = 0;
        for (const QVariant& day : activity)
            count += day.toBool() ? 1 : 0;
        return count;
    }

    TestApplication app;
};

TEST_F(TimeTravelTest, WorkoutLeavesWeekActivityAfterTravelingToNextWeek)
{
    app.setCurrentDate(QDate(2025, 1, 1));
    recordWorkout();

    const QVariantList activity = app.workoutHistoryViewModel().weekActivity();
    ASSERT_EQ(activity.size(), 7);
    EXPECT_TRUE(activity.at(2).toBool());
    EXPECT_EQ(activeDays(), 1);

    app.advanceDays(7);
    EXPECT_EQ(activeDays(), 0);
}

TEST_F(TimeTravelTest, RecordedWorkoutTimestampsFollowMockedClock)
{
    app.setCurrentDateTime(QDateTime(QDate(2024, 6, 10), QTime(9, 30, 0)));
    recordWorkout();

    auto* last = app.workoutHistoryViewModel().lastWorkout();
    ASSERT_NE(last, nullptr);
    EXPECT_EQ(last->startedTime().date(), QDate(2024, 6, 10));
    EXPECT_EQ(last->endedTime().date(), QDate(2024, 6, 10));
}

TEST_F(TimeTravelTest, TwoWorkoutsInDifferentWeeksCountSeparately)
{
    app.setCurrentDate(QDate(2025, 1, 1));
    recordWorkout();
    EXPECT_EQ(activeDays(), 1);

    app.advanceDays(7);
    recordWorkout();
    EXPECT_EQ(activeDays(), 1);

    app.advanceDays(7);
    EXPECT_EQ(activeDays(), 0);
}
