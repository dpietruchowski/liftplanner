#include "domain/workout/exercise.h"
#include "domain/workout/set.h"
#include "domain/workout/workout.h"
#include <gtest/gtest.h>

namespace
{

Exercise makeExercise(const QString& name) { return Exercise(name, 60); }

Set makeSet(int reps) { return Set(reps, 50.0); }

void expectDenseSetPositions(const Exercise& exercise)
{
    for (size_t i = 0; i < exercise.sets().size(); ++i)
        EXPECT_EQ(exercise.sets()[i].position(), static_cast<int>(i));
}

void expectDenseExercisePositions(const Workout& workout)
{
    for (size_t i = 0; i < workout.exercises().size(); ++i)
        EXPECT_EQ(workout.exercises()[i].position(), static_cast<int>(i));
}

}

class SetPositionTest : public ::testing::Test
{
};

TEST_F(SetPositionTest, DefaultPositionIsZero)
{
    Set set;
    EXPECT_EQ(set.position(), 0);
}

TEST_F(SetPositionTest, PositionIsSettable)
{
    Set set;
    set.setPosition(4);
    EXPECT_EQ(set.position(), 4);
}

class ExerciseSetOrderTest : public ::testing::Test
{
};

TEST_F(ExerciseSetOrderTest, AddSetAppendsAndNumbersFromZero)
{
    Exercise exercise = makeExercise("Squat");

    exercise.addSet(makeSet(5));
    exercise.addSet(makeSet(6));
    exercise.addSet(makeSet(7));

    ASSERT_EQ(exercise.sets().size(), 3u);
    expectDenseSetPositions(exercise);
    EXPECT_EQ(exercise.sets()[0].repetitions(), 5);
    EXPECT_EQ(exercise.sets()[2].repetitions(), 7);
}

TEST_F(ExerciseSetOrderTest, AddSetAtPositionInsertsInTheMiddle)
{
    Exercise exercise = makeExercise("Squat");
    exercise.addSet(makeSet(5));
    exercise.addSet(makeSet(7));

    exercise.addSet(makeSet(6), 1);

    ASSERT_EQ(exercise.sets().size(), 3u);
    EXPECT_EQ(exercise.sets()[0].repetitions(), 5);
    EXPECT_EQ(exercise.sets()[1].repetitions(), 6);
    EXPECT_EQ(exercise.sets()[2].repetitions(), 7);
    expectDenseSetPositions(exercise);
}

TEST_F(ExerciseSetOrderTest, AddSetBeyondSizeAppends)
{
    Exercise exercise = makeExercise("Squat");
    exercise.addSet(makeSet(5));

    exercise.addSet(makeSet(6), 99);

    ASSERT_EQ(exercise.sets().size(), 2u);
    EXPECT_EQ(exercise.sets()[1].repetitions(), 6);
    expectDenseSetPositions(exercise);
}

TEST_F(ExerciseSetOrderTest, RemoveSetRenumbersTheRest)
{
    Exercise exercise = makeExercise("Squat");
    exercise.addSet(makeSet(5));
    exercise.addSet(makeSet(6));
    exercise.addSet(makeSet(7));

    exercise.removeSet(0);

    ASSERT_EQ(exercise.sets().size(), 2u);
    EXPECT_EQ(exercise.sets()[0].repetitions(), 6);
    expectDenseSetPositions(exercise);
}

TEST_F(ExerciseSetOrderTest, RemoveSetOutOfRangeChangesNothing)
{
    Exercise exercise = makeExercise("Squat");
    exercise.addSet(makeSet(5));

    exercise.removeSet(-1);
    exercise.removeSet(7);

    EXPECT_EQ(exercise.sets().size(), 1u);
    expectDenseSetPositions(exercise);
}

TEST_F(ExerciseSetOrderTest, MoveSetForward)
{
    Exercise exercise = makeExercise("Squat");
    exercise.addSet(makeSet(5));
    exercise.addSet(makeSet(6));
    exercise.addSet(makeSet(7));

    exercise.moveSet(0, 2);

    EXPECT_EQ(exercise.sets()[0].repetitions(), 6);
    EXPECT_EQ(exercise.sets()[1].repetitions(), 7);
    EXPECT_EQ(exercise.sets()[2].repetitions(), 5);
    expectDenseSetPositions(exercise);
}

TEST_F(ExerciseSetOrderTest, MoveSetBackward)
{
    Exercise exercise = makeExercise("Squat");
    exercise.addSet(makeSet(5));
    exercise.addSet(makeSet(6));
    exercise.addSet(makeSet(7));

    exercise.moveSet(2, 0);

    EXPECT_EQ(exercise.sets()[0].repetitions(), 7);
    EXPECT_EQ(exercise.sets()[1].repetitions(), 5);
    EXPECT_EQ(exercise.sets()[2].repetitions(), 6);
    expectDenseSetPositions(exercise);
}

TEST_F(ExerciseSetOrderTest, MoveSetOutOfRangeChangesNothing)
{
    Exercise exercise = makeExercise("Squat");
    exercise.addSet(makeSet(5));
    exercise.addSet(makeSet(6));

    exercise.moveSet(0, 5);
    exercise.moveSet(-1, 1);
    exercise.moveSet(1, 1);

    EXPECT_EQ(exercise.sets()[0].repetitions(), 5);
    EXPECT_EQ(exercise.sets()[1].repetitions(), 6);
    expectDenseSetPositions(exercise);
}

class WorkoutExerciseOrderTest : public ::testing::Test
{
};

TEST_F(WorkoutExerciseOrderTest, AddExerciseAppendsAndNumbersFromZero)
{
    Workout workout("Push", QDateTime::currentDateTime());

    workout.addExercise(makeExercise("Bench"));
    workout.addExercise(makeExercise("Dip"));

    ASSERT_EQ(workout.exercises().size(), 2u);
    expectDenseExercisePositions(workout);
    EXPECT_EQ(workout.exercises()[0].name(), "Bench");
}

TEST_F(WorkoutExerciseOrderTest, AddExerciseAtPositionInsertsInTheMiddle)
{
    Workout workout("Push", QDateTime::currentDateTime());
    workout.addExercise(makeExercise("Bench"));
    workout.addExercise(makeExercise("Dip"));

    workout.addExercise(makeExercise("Fly"), 1);

    ASSERT_EQ(workout.exercises().size(), 3u);
    EXPECT_EQ(workout.exercises()[0].name(), "Bench");
    EXPECT_EQ(workout.exercises()[1].name(), "Fly");
    EXPECT_EQ(workout.exercises()[2].name(), "Dip");
    expectDenseExercisePositions(workout);
}

TEST_F(WorkoutExerciseOrderTest, RemoveExerciseRenumbersTheRest)
{
    Workout workout("Push", QDateTime::currentDateTime());
    workout.addExercise(makeExercise("Bench"));
    workout.addExercise(makeExercise("Fly"));
    workout.addExercise(makeExercise("Dip"));

    workout.removeExercise(1);

    ASSERT_EQ(workout.exercises().size(), 2u);
    EXPECT_EQ(workout.exercises()[1].name(), "Dip");
    expectDenseExercisePositions(workout);
}

TEST_F(WorkoutExerciseOrderTest, MoveExerciseReordersAndRenumbers)
{
    Workout workout("Push", QDateTime::currentDateTime());
    workout.addExercise(makeExercise("Bench"));
    workout.addExercise(makeExercise("Fly"));
    workout.addExercise(makeExercise("Dip"));

    workout.moveExercise(2, 0);

    EXPECT_EQ(workout.exercises()[0].name(), "Dip");
    EXPECT_EQ(workout.exercises()[1].name(), "Bench");
    EXPECT_EQ(workout.exercises()[2].name(), "Fly");
    expectDenseExercisePositions(workout);
}

TEST_F(WorkoutExerciseOrderTest, MoveExerciseOutOfRangeChangesNothing)
{
    Workout workout("Push", QDateTime::currentDateTime());
    workout.addExercise(makeExercise("Bench"));
    workout.addExercise(makeExercise("Fly"));

    workout.moveExercise(0, 9);
    workout.moveExercise(-3, 0);

    EXPECT_EQ(workout.exercises()[0].name(), "Bench");
    EXPECT_EQ(workout.exercises()[1].name(), "Fly");
    expectDenseExercisePositions(workout);
}

TEST_F(WorkoutExerciseOrderTest, NormalizePositionsRepairsDirectVectorEdits)
{
    Workout workout("Push", QDateTime::currentDateTime());
    Exercise bench = makeExercise("Bench");
    bench.addSet(makeSet(5));
    bench.addSet(makeSet(5));
    workout.addExercise(bench);
    workout.addExercise(makeExercise("Fly"));

    std::swap(workout.exercises()[0], workout.exercises()[1]);
    workout.exercises()[0].sets().push_back(makeSet(8));

    workout.normalizePositions();

    expectDenseExercisePositions(workout);
    for (const auto& exercise : workout.exercises())
        expectDenseSetPositions(exercise);
}

TEST_F(WorkoutExerciseOrderTest, NestedSetPositionsSurviveExerciseReorder)
{
    Workout workout("Push", QDateTime::currentDateTime());
    Exercise bench = makeExercise("Bench");
    bench.addSet(makeSet(5));
    bench.addSet(makeSet(6));
    workout.addExercise(makeExercise("Fly"));
    workout.addExercise(bench);

    workout.moveExercise(1, 0);

    EXPECT_EQ(workout.exercises()[0].name(), "Bench");
    expectDenseSetPositions(workout.exercises()[0]);
}
