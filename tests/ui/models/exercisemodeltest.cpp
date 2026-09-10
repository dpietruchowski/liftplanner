#include <gtest/gtest.h>

#include "domain/workout/exercise.h"
#include "ui/models/exercisemodel.h"

namespace
{
Exercise exerciseWithThreeSets()
{
    Exercise exercise = Exercise::createFromDefinition(11, QStringLiteral("Bench Press"),
                                                       ExerciseKind::Strength, 180);
    exercise.setId(5);
    exercise.setWorkoutId(3);
    exercise.setPosition(2);
    exercise.setDescription(QStringLiteral("Pause on the chest"));
    exercise.setNotes(QStringLiteral("Belt from set two"));
    exercise.addSet(Set(8, 60.0));
    exercise.addSet(Set(6, 70.0));
    exercise.addSet(Set(4, 80.0));
    return exercise;
}
}

TEST(ExerciseModelTest, ToEntity_CountsEverySetOnce)
{
    ExerciseModel model { exerciseWithThreeSets() };

    EXPECT_EQ(model.toEntity().sets().size(), 3u);
}

TEST(ExerciseModelTest, ToEntity_TakesTheSetsFromTheModels)
{
    ExerciseModel model { exerciseWithThreeSets() };

    model.sets().at(1)->setWeight(72.5);
    model.removeSet(model.sets().at(0));

    const Exercise entity = model.toEntity();
    ASSERT_EQ(entity.sets().size(), 2u);
    EXPECT_DOUBLE_EQ(entity.sets()[0].weight(), 72.5);
    EXPECT_DOUBLE_EQ(entity.sets()[1].weight(), 80.0);
}

TEST(ExerciseModelTest, ToEntity_KeepsEveryFieldOfTheExercise)
{
    const Exercise exercise = exerciseWithThreeSets();
    ExerciseModel model { exercise };

    const Exercise entity = model.toEntity();

    EXPECT_EQ(entity.id(), exercise.id());
    EXPECT_EQ(entity.workoutId(), exercise.workoutId());
    EXPECT_EQ(entity.name(), exercise.name());
    EXPECT_EQ(entity.description(), exercise.description());
    EXPECT_EQ(entity.notes(), exercise.notes());
    EXPECT_EQ(entity.restSeconds(), exercise.restSeconds());
    EXPECT_EQ(entity.kind(), exercise.kind());
    EXPECT_EQ(entity.position(), exercise.position());
    EXPECT_EQ(entity.definitionId(), exercise.definitionId());
}

TEST(ExerciseModelTest, PreviousPerformance_IsModelStateAndNeverReachesTheEntity)
{
    ExerciseModel model { exerciseWithThreeSets() };

    model.setPreviousPerformance(QStringLiteral("3x8 @ 60 kg"),
                                 QDateTime(QDate(2026, 2, 20), QTime(18, 0)),
                                 { QStringLiteral("8x60kg"), QString() });

    EXPECT_EQ(model.previousSummary(), QStringLiteral("3x8 @ 60 kg"));
    EXPECT_EQ(model.previousSetTexts(), QStringList({ QStringLiteral("8x60kg"), QString() }));
    EXPECT_EQ(model.toEntity().notes(), QStringLiteral("Belt from set two"));
}
