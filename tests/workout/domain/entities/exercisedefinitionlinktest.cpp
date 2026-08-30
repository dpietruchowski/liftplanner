#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/workout.h"
#include <gtest/gtest.h>

class ExerciseDefinitionLinkTest : public ::testing::Test
{
};

TEST_F(ExerciseDefinitionLinkTest, APlainExerciseCarriesNoDefinition)
{
    Exercise exercise("Zercher Squat", 120);

    EXPECT_FALSE(exercise.hasDefinition());
    EXPECT_FALSE(exercise.definitionId().has_value());
}

TEST_F(ExerciseDefinitionLinkTest, CreateFromDefinitionStoresTheIdAndSnapshotsTheName)
{
    const Exercise exercise
        = Exercise::createFromDefinition(42, "Barbell Back Squat", ExerciseKind::Strength, 180);

    EXPECT_TRUE(exercise.hasDefinition());
    EXPECT_EQ(exercise.definitionId().value(), 42);
    EXPECT_EQ(exercise.name(), "Barbell Back Squat");
    EXPECT_EQ(exercise.kind(), ExerciseKind::Strength);
    EXPECT_EQ(exercise.restSeconds(), 180);
}

TEST_F(ExerciseDefinitionLinkTest, AnAdHocExerciseIsStillFullyUsable)
{
    const Exercise exercise = Exercise::createAdHoc("Sandbag Carry", ExerciseKind::Cardio, 90);

    EXPECT_FALSE(exercise.hasDefinition());
    EXPECT_EQ(exercise.name(), "Sandbag Carry");
    EXPECT_EQ(exercise.kind(), ExerciseKind::Cardio);
    EXPECT_EQ(exercise.restSeconds(), 90);
}

TEST_F(ExerciseDefinitionLinkTest, TheLinkCanBeAttachedAndDetached)
{
    Exercise exercise("Bench Press", 120);

    exercise.setDefinitionId(7);
    EXPECT_EQ(exercise.definitionId().value(), 7);

    exercise.clearDefinitionId();
    EXPECT_FALSE(exercise.hasDefinition());
    EXPECT_EQ(exercise.name(), "Bench Press");
}

TEST_F(ExerciseDefinitionLinkTest, RenamingTheSnapshotLeavesTheLinkIntact)
{
    Exercise exercise
        = Exercise::createFromDefinition(42, "Barbell Back Squat", ExerciseKind::Strength, 180);

    exercise.setName("Przysiad ze sztanga");

    EXPECT_EQ(exercise.definitionId().value(), 42);
    EXPECT_EQ(exercise.name(), "Przysiad ze sztanga");
}

TEST_F(ExerciseDefinitionLinkTest, NotesAreSeparateFromTheMovementDescription)
{
    Exercise exercise("Bench Press", 120);

    exercise.setDescription("Flat barbell press off the chest");
    exercise.setNotes("Left shoulder felt tight");

    EXPECT_EQ(exercise.description(), "Flat barbell press off the chest");
    EXPECT_EQ(exercise.notes(), "Left shoulder felt tight");
}

TEST_F(ExerciseDefinitionLinkTest, NotesDefaultToEmpty)
{
    Exercise exercise("Bench Press", 120);

    EXPECT_TRUE(exercise.notes().isEmpty());
}

TEST_F(ExerciseDefinitionLinkTest, CopyingKeepsTheLinkAndTheNotes)
{
    Exercise original
        = Exercise::createFromDefinition(42, "Barbell Back Squat", ExerciseKind::Strength, 180);
    original.setNotes("Belt on from 100kg");

    const Exercise copy = original;

    EXPECT_EQ(copy.definitionId().value(), 42);
    EXPECT_EQ(copy.notes(), "Belt on from 100kg");
}

TEST_F(ExerciseDefinitionLinkTest, LinkedAndAdHocExercisesLiveInTheSameWorkout)
{
    Workout workout("Mixed", QDateTime::currentDateTime());

    workout.addExercise(
        Exercise::createFromDefinition(42, "Barbell Back Squat", ExerciseKind::Strength, 180));
    workout.addExercise(Exercise::createAdHoc("Sandbag Carry", ExerciseKind::Cardio, 90));

    ASSERT_EQ(workout.exercises().size(), 2u);
    EXPECT_TRUE(workout.exercises()[0].hasDefinition());
    EXPECT_FALSE(workout.exercises()[1].hasDefinition());
    EXPECT_EQ(workout.exercises()[0].position(), 0);
    EXPECT_EQ(workout.exercises()[1].position(), 1);
}
