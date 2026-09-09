#include "domain/workout/exercise.h"
#include "domain/workout/set.h"
#include "domain/workout/setcompatibility.h"
#include "domain/workout/workout.h"
#include <gtest/gtest.h>

namespace
{

Exercise strengthExercise()
{
    Exercise exercise = Exercise::createAdHoc("Bench Press", ExerciseKind::Strength, 120);
    exercise.addSet(Set(5, 60.0));
    exercise.addSet(Set(5, 70.0));
    return exercise;
}

Workout plannedWorkout()
{
    Workout workout("Push", QDateTime::currentDateTime());
    workout.addExercise(strengthExercise());
    return workout;
}

}

class DuplicateSetTest : public ::testing::Test
{
};

TEST_F(DuplicateSetTest, TheCopyLandsRightAfterTheOriginal)
{
    Exercise exercise = strengthExercise();

    exercise.duplicateSet(0);

    ASSERT_EQ(exercise.sets().size(), 3u);
    EXPECT_EQ(exercise.sets()[0].weight(), 60.0);
    EXPECT_EQ(exercise.sets()[1].weight(), 60.0);
    EXPECT_EQ(exercise.sets()[2].weight(), 70.0);
    for (size_t i = 0; i < exercise.sets().size(); ++i)
        EXPECT_EQ(exercise.sets()[i].position(), static_cast<int>(i));
}

TEST_F(DuplicateSetTest, TheCopyStartsUncompletedAndUnsaved)
{
    Exercise exercise = strengthExercise();
    exercise.sets()[0].setCompleted(true);
    exercise.sets()[0].setId(99);

    exercise.duplicateSet(0);

    EXPECT_TRUE(exercise.sets()[0].completed());
    EXPECT_FALSE(exercise.sets()[1].completed());
    EXPECT_EQ(exercise.sets()[1].id(), -1);
}

TEST_F(DuplicateSetTest, OutOfRangeChangesNothing)
{
    Exercise exercise = strengthExercise();

    exercise.duplicateSet(-1);
    exercise.duplicateSet(9);

    EXPECT_EQ(exercise.sets().size(), 2u);
}

class SetMetricCompatibilityTest : public ::testing::Test
{
};

TEST_F(SetMetricCompatibilityTest, StrengthAcceptsRepsOnly)
{
    const Exercise exercise = Exercise::createAdHoc("Bench", ExerciseKind::Strength, 120);

    EXPECT_TRUE(exercise.acceptsSet(Set(5, 60.0)));
    EXPECT_FALSE(exercise.acceptsSet(Set::createDuration(60)));
    EXPECT_FALSE(exercise.acceptsSet(Set::createDistance(1000.0, 300)));
}

TEST_F(SetMetricCompatibilityTest, CardioAcceptsDurationAndDistance)
{
    const Exercise exercise = Exercise::createAdHoc("Run", ExerciseKind::Cardio, 60);

    EXPECT_TRUE(exercise.acceptsSet(Set::createDuration(600)));
    EXPECT_TRUE(exercise.acceptsSet(Set::createDistance(5000.0, 1500)));
    EXPECT_FALSE(exercise.acceptsSet(Set(10, 0.0)));
}

TEST_F(SetMetricCompatibilityTest, IntervalAcceptsDurationAndReps)
{
    const Exercise exercise = Exercise::createAdHoc("Burpees", ExerciseKind::Interval, 30);

    EXPECT_TRUE(exercise.acceptsSet(Set::createDuration(45)));
    EXPECT_TRUE(exercise.acceptsSet(Set(15, 0.0)));
    EXPECT_FALSE(exercise.acceptsSet(Set::createDistance(200.0, 60)));
}

TEST_F(SetMetricCompatibilityTest, IsometricAcceptsDurationOnly)
{
    const Exercise exercise = Exercise::createAdHoc("Plank", ExerciseKind::Isometric, 60);

    EXPECT_TRUE(exercise.acceptsSet(Set::createDuration(60)));
    EXPECT_FALSE(exercise.acceptsSet(Set(15, 0.0)));
    EXPECT_FALSE(exercise.acceptsSet(Set::createDistance(200.0, 60)));
    EXPECT_EQ(defaultMetricFor(ExerciseKind::Isometric), SetMetric::Duration);
    EXPECT_EQ(defaultLoadTypeFor(ExerciseKind::Isometric), LoadType::None);
}

TEST_F(SetMetricCompatibilityTest, AnIncompatibleSetIsReportedButNeverDropped)
{
    Exercise exercise = Exercise::createAdHoc("Run", ExerciseKind::Cardio, 60);

    exercise.addSet(Set(10, 20.0));

    EXPECT_EQ(exercise.sets().size(), 1u);
    EXPECT_FALSE(exercise.isValid());
    EXPECT_EQ(exercise.validationErrors().size(), 1);
}

class ExerciseValidationTest : public ::testing::Test
{
};

TEST_F(ExerciseValidationTest, AWellFormedExerciseIsValid)
{
    EXPECT_TRUE(strengthExercise().isValid());
}

TEST_F(ExerciseValidationTest, NameMustNotBeEmpty)
{
    Exercise exercise = strengthExercise();
    exercise.setName("  ");

    EXPECT_FALSE(exercise.isValid());
}

TEST_F(ExerciseValidationTest, RestSecondsMustNotBeNegative)
{
    Exercise exercise = strengthExercise();
    exercise.setRestSeconds(-5);

    EXPECT_FALSE(exercise.isValid());
}

class WorkoutValidationTest : public ::testing::Test
{
};

TEST_F(WorkoutValidationTest, AWellFormedWorkoutIsValid)
{
    EXPECT_TRUE(plannedWorkout().isValid());
}

TEST_F(WorkoutValidationTest, ErrorsFromExercisesAreReportedWithTheirIndex)
{
    Workout workout = plannedWorkout();
    workout.addExercise(Exercise::createAdHoc("", ExerciseKind::Strength, 120));

    ASSERT_FALSE(workout.isValid());
    EXPECT_TRUE(workout.validationErrors().first().startsWith("exercise 1:"));
}

TEST_F(WorkoutValidationTest, AnEmptyWorkoutIsStillValidButKnowsItIsEmpty)
{
    Workout workout("Push", QDateTime::currentDateTime());

    EXPECT_TRUE(workout.isEmpty());
    EXPECT_TRUE(workout.isValid());
}

TEST_F(WorkoutValidationTest, WorkoutNameMustNotBeEmpty)
{
    Workout workout("   ", QDateTime::currentDateTime());

    EXPECT_FALSE(workout.isValid());
}

class EditingDoesNotChangeStatusTest : public ::testing::Test
{
};

TEST_F(EditingDoesNotChangeStatusTest, EditingAPlannedWorkoutKeepsItPlanned)
{
    Workout workout = plannedWorkout();
    ASSERT_EQ(workout.status(), WorkoutStatus::Planned);

    workout.addExercise(strengthExercise());
    workout.moveExercise(1, 0);
    workout.removeExercise(0);

    EXPECT_EQ(workout.status(), WorkoutStatus::Planned);
}

TEST_F(EditingDoesNotChangeStatusTest, CorrectingAnEndedWorkoutDoesNotReopenIt)
{
    Workout workout = plannedWorkout();
    workout.start();
    workout.end();
    const QDateTime endedAt = workout.endedTime();

    workout.exercises()[0].sets()[0].setWeight(62.5);
    workout.addExercise(Exercise::createAdHoc("Dip", ExerciseKind::Bodyweight, 90));

    EXPECT_EQ(workout.status(), WorkoutStatus::Ended);
    EXPECT_EQ(workout.endedTime(), endedAt);
}

TEST_F(EditingDoesNotChangeStatusTest, EditingAStartedWorkoutKeepsTheStartTime)
{
    Workout workout = plannedWorkout();
    workout.start();
    const QDateTime startedAt = workout.startedTime();

    workout.exercises()[0].duplicateSet(0);
    workout.addExercise(strengthExercise(), 0);

    EXPECT_EQ(workout.status(), WorkoutStatus::Started);
    EXPECT_EQ(workout.startedTime(), startedAt);
}
