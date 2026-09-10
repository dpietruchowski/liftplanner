#include <gtest/gtest.h>

#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

// =============================================================================
// Home tiles: 1RM belongs to weighted lifts, timed and covered work get
// their own tiles instead of being reported as "0 kg"
// =============================================================================

class HomeStatsTest : public ::testing::Test
{
protected:
    void recordWorkout(const QString& json)
    {
        app.plannedWorkoutViewModel().importFromJson(json);
        app.plannedWorkoutViewModel().loadAll();
        app.drain();

        auto& active = app.activeWorkoutViewModel();
        active.startWorkout(app.plannedWorkoutViewModel().workouts().first());

        for (int guard = 0; guard < 64 && !active.currentWorkout()->isCompleted(); ++guard)
            active.completeCurrentSet();

        active.endWorkout();

        app.workoutHistoryViewModel().loadAllWorkouts();
        app.drain();
    }

    QStringList tileNames()
    {
        QStringList names;
        for (const QVariant& tile : app.workoutHistoryViewModel().topExercises())
            names.append(tile.toMap()["name"].toString());
        return names;
    }

    QVariantMap totalsTile(const QString& label)
    {
        for (const QVariant& tile : app.workoutHistoryViewModel().recentTotals())
        {
            const QVariantMap map = tile.toMap();
            if (map["label"].toString() == label)
                return map;
        }
        return QVariantMap();
    }

    TestApplication app;
};

TEST_F(HomeStatsTest, OneRepMaxTilesSkipTimedDistanceAndBodyweightWork)
{
    recordWorkout(TestData::MIXED_WORKOUT_JSON);

    const QStringList names = tileNames();

    EXPECT_TRUE(names.contains("Bench Press"));
    EXPECT_FALSE(names.contains("Plank"));
    EXPECT_FALSE(names.contains("Run"));
    EXPECT_FALSE(names.contains("Burpee intervals"));
    EXPECT_FALSE(names.contains("Pull-ups"));
}

TEST_F(HomeStatsTest, OneRepMaxTileCarriesANonZeroValue)
{
    recordWorkout(TestData::MIXED_WORKOUT_JSON);

    const QVariantList tiles = app.workoutHistoryViewModel().topExercises();
    ASSERT_FALSE(tiles.isEmpty());

    for (const QVariant& tile : tiles)
        EXPECT_GT(tile.toMap()["oneRepMax"].toDouble(), 0.0);
}

TEST_F(HomeStatsTest, TimeAndDistanceGetTheirOwnTiles)
{
    recordWorkout(TestData::MIXED_WORKOUT_JSON);

    // 45s + 45s + 3min planks, 8 burpee rounds of 20s, and a 24min 5km run.
    EXPECT_EQ(totalsTile("time")["value"].toString(), "31m");
    EXPECT_EQ(totalsTile("distance")["value"].toString(), "5 km");
}

TEST_F(HomeStatsTest, ALifterWhoNeverWorksAgainstTheClockStillGetsAStatTile)
{
    recordWorkout(TestData::SINGLE_WORKOUT_JSON);

    EXPECT_FALSE(app.workoutHistoryViewModel().recentTotals().isEmpty());
    EXPECT_EQ(totalsTile("volume")["value"].toString(), "900 kg");
    EXPECT_TRUE(totalsTile("time").isEmpty());
    EXPECT_TRUE(totalsTile("distance").isEmpty());
}

TEST_F(HomeStatsTest, VolumeAccumulatesAcrossWorkouts)
{
    recordWorkout(TestData::SINGLE_WORKOUT_JSON);
    recordWorkout(TestData::SINGLE_WORKOUT_JSON);

    EXPECT_EQ(totalsTile("volume")["value"].toString(), "1 800 kg");
}

TEST_F(HomeStatsTest, AnEmptyHistoryReportsNoTilesAtAll)
{
    app.workoutHistoryViewModel().loadAllWorkouts();
    app.drain();

    EXPECT_TRUE(app.workoutHistoryViewModel().recentTotals().isEmpty());
}

TEST_F(HomeStatsTest, TotalsAccumulateAcrossWorkouts)
{
    recordWorkout(TestData::MIXED_WORKOUT_JSON);
    recordWorkout(TestData::MIXED_WORKOUT_JSON);

    EXPECT_EQ(totalsTile("time")["value"].toString(), "1h 02m");
    EXPECT_EQ(totalsTile("distance")["value"].toString(), "10 km");
}
