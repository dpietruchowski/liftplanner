#include <gtest/gtest.h>

#include "domain/workout/exercise.h"
#include "domain/workout/sessionsummary.h"
#include "domain/workout/set.h"
#include "domain/workout/workout.h"
#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"

namespace
{

Set tickedSet(int repetitions, double weight)
{
    Set set(repetitions, weight);
    set.setCompleted(true);
    return set;
}

Exercise exerciseWith(const QString& name, std::initializer_list<Set> sets)
{
    Exercise exercise(name, 120);
    for (const Set& set : sets)
        exercise.addSet(set);
    return exercise;
}

Workout sessionWith(std::initializer_list<Exercise> exercises)
{
    Workout workout(QStringLiteral("Summary check"), QDateTime(QDate(2026, 3, 1), QTime(18, 0)));
    for (const Exercise& exercise : exercises)
        workout.addExercise(exercise);
    return workout;
}

bool chipPerformed(const WorkoutModel& workout, int exerciseIndex, int setIndex)
{
    const SetModel* set = workout.exercises().at(exerciseIndex)->sets().at(setIndex);
    return set->performed(workout.completionFlagsMeaningful());
}

}  // namespace

TEST(SetPerformanceTest, ASessionWithATickHasMeaningfulFlags)
{
    WorkoutModel workout { sessionWith(
        { exerciseWith(QStringLiteral("Back Squat"), { tickedSet(5, 40.0), Set(5, 40.0) }) }) };

    EXPECT_TRUE(workout.completionFlagsMeaningful());
}

TEST(SetPerformanceTest, TheTickedSetIsPerformedAndTheOtherIsNot)
{
    WorkoutModel workout { sessionWith(
        { exerciseWith(QStringLiteral("Back Squat"), { tickedSet(5, 40.0), Set(5, 40.0) }) }) };

    EXPECT_TRUE(chipPerformed(workout, 0, 0));
    EXPECT_FALSE(chipPerformed(workout, 0, 1));
}

TEST(SetPerformanceTest, AnExerciseWithoutTicksIsSkippedWhenAnotherOneHasThem)
{
    WorkoutModel workout { sessionWith(
        { exerciseWith(QStringLiteral("Back Squat"), { tickedSet(5, 40.0) }),
          exerciseWith(QStringLiteral("Bench Press"), { Set(5, 30.0), Set(5, 30.0) }) }) };

    EXPECT_TRUE(chipPerformed(workout, 0, 0));
    EXPECT_FALSE(chipPerformed(workout, 1, 0));
    EXPECT_FALSE(chipPerformed(workout, 1, 1));
}

TEST(SetPerformanceTest, AnImportedSessionWithoutASingleTickCountsAsDone)
{
    WorkoutModel workout { sessionWith(
        { exerciseWith(QStringLiteral("Back Squat"), { Set(5, 40.0), Set(5, 40.0) }),
          exerciseWith(QStringLiteral("Bench Press"), { Set(5, 30.0) }) }) };

    EXPECT_FALSE(workout.completionFlagsMeaningful());
    EXPECT_TRUE(chipPerformed(workout, 0, 0));
    EXPECT_TRUE(chipPerformed(workout, 0, 1));
    EXPECT_TRUE(chipPerformed(workout, 1, 0));
}

TEST(SetPerformanceTest, APlannedWorkoutMarksNothingAsSkipped)
{
    Workout planned(QStringLiteral("Push A"), QDateTime(QDate(2026, 3, 1), QTime(18, 0)));
    planned.addExercise(exerciseWith(QStringLiteral("Back Squat"), { Set(5, 40.0), Set(5, 40.0) }));
    WorkoutModel workout { planned };

    EXPECT_FALSE(workout.completionFlagsMeaningful());
    EXPECT_TRUE(chipPerformed(workout, 0, 0));
    EXPECT_TRUE(chipPerformed(workout, 0, 1));
}

TEST(SetPerformanceTest, TickingASetTurnsTheRestOfTheSessionIntoSkipped)
{
    WorkoutModel workout { sessionWith(
        { exerciseWith(QStringLiteral("Back Squat"), { Set(5, 40.0), Set(5, 40.0) }) }) };
    ASSERT_TRUE(chipPerformed(workout, 0, 1));

    workout.exercises().first()->sets().first()->setCompleted(true);

    EXPECT_TRUE(workout.completionFlagsMeaningful());
    EXPECT_TRUE(chipPerformed(workout, 0, 0));
    EXPECT_FALSE(chipPerformed(workout, 0, 1));
}

TEST(SetPerformanceTest, TheChipAgreesWithTheSessionSummaryOfTheSameWorkout)
{
    WorkoutModel workout { sessionWith(
        { exerciseWith(QStringLiteral("Back Squat"), { tickedSet(5, 40.0), Set(5, 40.0) }) }) };

    int performedChips = 0;
    for (const SetModel* set : workout.exercises().first()->sets())
        performedChips += set->performed(workout.completionFlagsMeaningful()) ? 1 : 0;

    EXPECT_EQ(performedChips, summarizeSession(workout.toEntity()).completedSets);
}
