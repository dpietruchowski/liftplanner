#include "domain/workout/workoutrepeat.h"
#include <gtest/gtest.h>

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
    Exercise exercise(name, 90);
    for (const Set& set : sets)
        exercise.addSet(set);
    return exercise;
}

Workout performedSession()
{
    Workout workout(QStringLiteral("Monday Heavy"), QDateTime(QDate(2025, 5, 5), QTime(17, 0, 0)));
    workout.setId(7);
    workout.setStatus(WorkoutStatus::Ended);
    workout.setStartedTime(QDateTime(QDate(2025, 5, 5), QTime(17, 0, 0)));
    workout.setEndedTime(QDateTime(QDate(2025, 5, 5), QTime(18, 10, 0)));

    Exercise squat = exerciseWith(QStringLiteral("Back Squat"),
                                  { tickedSet(5, 100.0), tickedSet(5, 100.0), Set(5, 100.0) });
    squat.setId(11);
    squat.setWorkoutId(7);
    squat.setDefinitionId(3);
    squat.setNotes(QStringLiteral("belt on the last one"));

    Exercise bench = exerciseWith(QStringLiteral("Bench Press"), { tickedSet(8, 60.0) });
    bench.setId(12);
    bench.setWorkoutId(7);

    workout.addExercise(squat);
    workout.addExercise(bench);
    return workout;
}

const QDateTime today { QDate(2025, 5, 12), QTime(9, 0, 0) };

}  // namespace

class WorkoutRepeatTest : public ::testing::Test
{
};

TEST_F(WorkoutRepeatTest, TheCopyHasTheSameExercisesAndSets)
{
    const Workout repeated = repeatOf(performedSession(), today);

    ASSERT_EQ(repeated.exercises().size(), 2u);
    EXPECT_EQ(repeated.exercises()[0].name(), QStringLiteral("Back Squat"));
    EXPECT_EQ(repeated.exercises()[1].name(), QStringLiteral("Bench Press"));
    ASSERT_EQ(repeated.exercises()[0].sets().size(), 3u);
    ASSERT_EQ(repeated.exercises()[1].sets().size(), 1u);
    EXPECT_EQ(repeated.totalSets(), 4);
}

TEST_F(WorkoutRepeatTest, TheCopyKeepsTheWeightsAndRepetitionsOfThatSession)
{
    const Workout repeated = repeatOf(performedSession(), today);

    const Set& squatSet = repeated.exercises()[0].sets().front();
    EXPECT_EQ(squatSet.repetitions(), 5);
    EXPECT_DOUBLE_EQ(squatSet.weight(), 100.0);

    const Set& benchSet = repeated.exercises()[1].sets().front();
    EXPECT_EQ(benchSet.repetitions(), 8);
    EXPECT_DOUBLE_EQ(benchSet.weight(), 60.0);
}

TEST_F(WorkoutRepeatTest, NotASingleSetOfTheCopyIsTickedOff)
{
    const Workout repeated = repeatOf(performedSession(), today);

    for (const Exercise& exercise : repeated.exercises())
        for (const Set& set : exercise.sets())
            EXPECT_FALSE(set.completed());

    EXPECT_FALSE(repeated.isCompleted());
}

TEST_F(WorkoutRepeatTest, TheCopyIsPlannedForTheGivenDayAndNeverStarted)
{
    const Workout repeated = repeatOf(performedSession(), today);

    EXPECT_EQ(repeated.status(), WorkoutStatus::Planned);
    EXPECT_EQ(repeated.plannedTime(), today);
    EXPECT_FALSE(repeated.startedTime().isValid());
    EXPECT_FALSE(repeated.endedTime().isValid());
}

TEST_F(WorkoutRepeatTest, TheCopyIsANewRowNotTheOriginalOne)
{
    const Workout repeated = repeatOf(performedSession(), today);

    EXPECT_EQ(repeated.id(), -1);
    for (const Exercise& exercise : repeated.exercises())
    {
        EXPECT_EQ(exercise.id(), -1);
        EXPECT_EQ(exercise.workoutId(), -1);
    }
}

TEST_F(WorkoutRepeatTest, TheCopyStaysLinkedToTheExerciseCatalog)
{
    const Workout repeated = repeatOf(performedSession(), today);

    ASSERT_TRUE(repeated.exercises()[0].hasDefinition());
    EXPECT_EQ(repeated.exercises()[0].definitionId().value(), 3);
    EXPECT_EQ(repeated.exercises()[0].restSeconds(), 90);
    EXPECT_EQ(repeated.exercises()[0].notes(), QStringLiteral("belt on the last one"));
}

TEST_F(WorkoutRepeatTest, TheOriginalSessionIsLeftUntouched)
{
    const Workout source = performedSession();
    Workout original = source;

    repeatOf(original, today);

    EXPECT_EQ(original.id(), 7);
    EXPECT_EQ(original.status(), WorkoutStatus::Ended);
    EXPECT_EQ(original.startedTime(), source.startedTime());
    EXPECT_EQ(original.endedTime(), source.endedTime());
    ASSERT_EQ(original.exercises().size(), 2u);
    EXPECT_EQ(original.exercises()[0].id(), 11);
    EXPECT_TRUE(original.exercises()[0].sets()[0].completed());
    EXPECT_TRUE(original.exercises()[1].sets()[0].completed());
}

TEST_F(WorkoutRepeatTest, ASkippedSetIsPartOfThePlanTheLifterRepeats)
{
    const Workout repeated = repeatOf(performedSession(), today);

    EXPECT_EQ(repeated.exercises()[0].sets().size(), 3u);
}

TEST_F(WorkoutRepeatTest, TimedAndDistanceWorkSurvivesTheCopy)
{
    Workout source(QStringLiteral("Conditioning"), QDateTime(QDate(2025, 5, 5), QTime(7, 0, 0)));
    Set row = Set::createDistance(2000.0, 480);
    row.setCompleted(true);
    source.addExercise(exerciseWith(QStringLiteral("Row"), { row }));

    const Workout repeated = repeatOf(source, today);

    const Set& copied = repeated.exercises()[0].sets().front();
    EXPECT_EQ(copied.metric(), row.metric());
    EXPECT_DOUBLE_EQ(copied.distanceMeters(), 2000.0);
    EXPECT_EQ(copied.durationSeconds(), 480);
    EXPECT_FALSE(copied.completed());
}
