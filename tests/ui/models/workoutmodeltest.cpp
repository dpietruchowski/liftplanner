#include <QSignalSpy>
#include <gtest/gtest.h>

#include "domain/workout/workout.h"
#include "ui/models/workoutmodel.h"

TEST(WorkoutModelTest, NewWorkout_HasNoId)
{
    WorkoutModel model { Workout(QStringLiteral("Push Day"), QDateTime::currentDateTime()) };

    EXPECT_EQ(model.id(), -1);
}

TEST(WorkoutModelTest, SetId_ReportsTheNewId)
{
    WorkoutModel model { Workout(QStringLiteral("Push Day"), QDateTime::currentDateTime()) };

    model.setId(42);

    EXPECT_EQ(model.id(), 42);
}

TEST(WorkoutModelTest, SetId_NotifiesTheBinding)
{
    WorkoutModel model { Workout(QStringLiteral("Push Day"), QDateTime::currentDateTime()) };
    QSignalSpy spy(&model, &WorkoutModel::dataChanged);

    model.setId(42);

    EXPECT_EQ(spy.count(), 1);
}

TEST(WorkoutModelTest, SetId_ToTheSameId_NotifiesNothing)
{
    Workout workout(QStringLiteral("Push Day"), QDateTime::currentDateTime());
    workout.setId(42);
    WorkoutModel model { workout };
    QSignalSpy spy(&model, &WorkoutModel::dataChanged);

    model.setId(42);

    EXPECT_EQ(spy.count(), 0);
}
