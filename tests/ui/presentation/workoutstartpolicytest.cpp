#include <gtest/gtest.h>

#include "domain/workout/workout.h"
#include "ui/models/workoutmodel.h"
#include "ui/presentation/workoutstartpolicy.h"

namespace
{

const QDateTime baseTime { QDate(2026, 1, 5), QTime(18, 0) };

Workout plannedWorkout(const QString& name, int id = 0)
{
    Workout workout(name, baseTime);
    workout.setPlannedTime(baseTime);
    if (id != 0)
        workout.setId(id);
    return workout;
}

Workout finishedWorkout(const QString& name, const QDateTime& endedTime)
{
    Workout workout(name, baseTime);
    workout.setStartedTime(endedTime.addSecs(-3600));
    workout.setEndedTime(endedTime);
    workout.setStatus(WorkoutStatus::Ended);
    return workout;
}

QString actionOf(const QVariantMap& decision)
{
    return decision.value(QString::fromLatin1(WorkoutStartPolicy::actionKey)).toString();
}

QString labelOf(const QVariantMap& decision)
{
    return decision.value(QString::fromLatin1(WorkoutStartPolicy::labelKey)).toString();
}

QString messageOf(const QVariantMap& decision)
{
    return decision.value(QString::fromLatin1(WorkoutStartPolicy::messageKey)).toString();
}

}  // namespace

TEST(WorkoutStartPolicyTest, EmptySlot_AsksForAPlan)
{
    WorkoutStartPolicy policy;

    const QVariantMap decision = policy.decide(nullptr, nullptr);

    EXPECT_EQ(actionOf(decision), QStringLiteral("missing"));
}

TEST(WorkoutStartPolicyTest, PlannedWorkout_WithoutActiveOne_Starts)
{
    WorkoutStartPolicy policy;
    WorkoutModel selected { plannedWorkout(QStringLiteral("Push B")) };

    const QVariantMap decision = policy.decide(&selected, nullptr);

    EXPECT_EQ(actionOf(decision), QStringLiteral("start"));
    EXPECT_EQ(labelOf(decision), QStringLiteral("Start workout"));
    EXPECT_TRUE(messageOf(decision).isEmpty());
}

TEST(WorkoutStartPolicyTest, OtherPlannedWorkout_WhileOneRuns_AsksToReplace)
{
    WorkoutStartPolicy policy;
    WorkoutModel selected { plannedWorkout(QStringLiteral("Push B"), 7) };
    WorkoutModel active { plannedWorkout(QStringLiteral("Push A"), 3) };

    const QVariantMap decision = policy.decide(&selected, &active);

    EXPECT_EQ(actionOf(decision), QStringLiteral("replace"));
}

TEST(WorkoutStartPolicyTest, ActiveWorkoutItself_Resumes)
{
    WorkoutStartPolicy policy;
    WorkoutModel active { plannedWorkout(QStringLiteral("Push A"), 3) };

    const QVariantMap decision = policy.decide(&active, &active);

    EXPECT_EQ(actionOf(decision), QStringLiteral("resume"));
    EXPECT_EQ(labelOf(decision), QStringLiteral("Continue workout"));
}

TEST(WorkoutStartPolicyTest, CopyOfTheActiveWorkout_Resumes)
{
    WorkoutStartPolicy policy;
    WorkoutModel selected { plannedWorkout(QStringLiteral("Push A"), 3) };
    WorkoutModel active { plannedWorkout(QStringLiteral("Push A"), 3) };

    const QVariantMap decision = policy.decide(&selected, &active);

    EXPECT_EQ(actionOf(decision), QStringLiteral("resume"));
}

TEST(WorkoutStartPolicyTest, UnsavedWorkouts_AreNotConfusedByTheirMissingIds)
{
    WorkoutStartPolicy policy;
    WorkoutModel selected { plannedWorkout(QStringLiteral("Push B")) };
    WorkoutModel active { plannedWorkout(QStringLiteral("Push A")) };

    const QVariantMap decision = policy.decide(&selected, &active);

    EXPECT_EQ(actionOf(decision), QStringLiteral("replace"));
}

TEST(WorkoutStartPolicyTest, FinishedWorkout_IsBlockedAndExplained)
{
    WorkoutStartPolicy policy;
    WorkoutModel selected { finishedWorkout(QStringLiteral("Leg Day"),
                                            QDateTime(QDate(2026, 1, 3), QTime(19, 30))) };

    const QVariantMap decision = policy.decide(&selected, nullptr);

    EXPECT_EQ(actionOf(decision), QStringLiteral("blocked"));
    EXPECT_TRUE(messageOf(decision).contains(QStringLiteral("Leg Day")));
    EXPECT_TRUE(messageOf(decision).contains(QStringLiteral("3 Jan 2026")));
    EXPECT_TRUE(messageOf(decision).contains(QStringLiteral("History")));
}

TEST(WorkoutStartPolicyTest, FinishedWorkout_WhileAnotherIsPlanned_StillBlocked)
{
    WorkoutStartPolicy policy;
    WorkoutModel selected { finishedWorkout(QStringLiteral("Leg Day"),
                                            QDateTime(QDate(2026, 1, 3), QTime(19, 30))) };
    WorkoutModel active { plannedWorkout(QStringLiteral("Push A"), 3) };

    const QVariantMap decision = policy.decide(&selected, &active);

    EXPECT_EQ(actionOf(decision), QStringLiteral("blocked"));
}

TEST(WorkoutStartPolicyTest, WorkoutWithEndTimeButStalePlannedStatus_IsBlocked)
{
    Workout workout = plannedWorkout(QStringLiteral("Leg Day"), 9);
    workout.setEndedTime(QDateTime(QDate(2026, 1, 3), QTime(19, 30)));
    WorkoutStartPolicy policy;
    WorkoutModel selected { workout };

    const QVariantMap decision = policy.decide(&selected, nullptr);

    EXPECT_EQ(actionOf(decision), QStringLiteral("blocked"));
}

TEST(WorkoutStartPolicyTest, EpochEndTime_DoesNotBlockAPlannedWorkout)
{
    Workout workout = plannedWorkout(QStringLiteral("Push B"), 11);
    workout.setEndedTime(QDateTime::fromMSecsSinceEpoch(0));
    WorkoutStartPolicy policy;
    WorkoutModel selected { workout };

    const QVariantMap decision = policy.decide(&selected, nullptr);

    EXPECT_EQ(actionOf(decision), QStringLiteral("start"));
}

TEST(WorkoutStartPolicyTest, UnnamedFinishedWorkout_IsStillExplained)
{
    WorkoutStartPolicy policy;
    WorkoutModel selected { finishedWorkout(QString(),
                                            QDateTime(QDate(2026, 1, 3), QTime(19, 30))) };

    const QVariantMap decision = policy.decide(&selected, nullptr);

    EXPECT_EQ(actionOf(decision), QStringLiteral("blocked"));
    EXPECT_TRUE(messageOf(decision).startsWith(QStringLiteral("That workout")));
}
