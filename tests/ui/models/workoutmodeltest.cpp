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

namespace
{
Workout workoutWithTwoExercises()
{
    Workout workout(QStringLiteral("Push Day"), QDateTime::currentDateTime());
    workout.addExercise(Exercise(QStringLiteral("Bench Press"), 180));
    workout.addExercise(Exercise(QStringLiteral("Overhead Press"), 120));
    return workout;
}
}

TEST(WorkoutModelTest, ToEntity_CountsEveryExerciseOnce)
{
    WorkoutModel model { workoutWithTwoExercises() };

    EXPECT_EQ(model.toEntity().exercises().size(), 2u);
}

TEST(WorkoutModelTest, ToEntity_TakesTheExercisesFromTheModels)
{
    WorkoutModel model { workoutWithTwoExercises() };

    model.moveExercise(0, 1);

    const Workout entity = model.toEntity();
    ASSERT_EQ(entity.exercises().size(), 2u);
    EXPECT_EQ(entity.exercises()[0].name(), QStringLiteral("Overhead Press"));
    EXPECT_EQ(entity.exercises()[1].name(), QStringLiteral("Bench Press"));
}

TEST(WorkoutModelTest, ToEntity_KeepsEveryFieldOfTheWorkout)
{
    Workout workout(QStringLiteral("Push Day"), QDateTime(QDate(2026, 3, 1), QTime(7, 0)));
    workout.setId(7);
    workout.setPlannedTime(QDateTime(QDate(2026, 3, 2), QTime(18, 0)));
    workout.setStartedTime(QDateTime(QDate(2026, 3, 2), QTime(18, 5)));
    workout.setEndedTime(QDateTime(QDate(2026, 3, 2), QTime(19, 10)));
    workout.setStatus(WorkoutStatus::Ended);
    WorkoutModel model { workout };

    const Workout entity = model.toEntity();

    EXPECT_EQ(entity.id(), workout.id());
    EXPECT_EQ(entity.name(), workout.name());
    EXPECT_EQ(entity.createdTime(), workout.createdTime());
    EXPECT_EQ(entity.plannedTime(), workout.plannedTime());
    EXPECT_EQ(entity.startedTime(), workout.startedTime());
    EXPECT_EQ(entity.endedTime(), workout.endedTime());
    EXPECT_EQ(entity.status(), workout.status());
}
