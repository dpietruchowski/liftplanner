#include "domain/workout/workouttemplate.h"
#include "domain/workout/setprescription.h"
#include "domain/workout/templateexercise.h"
#include <gtest/gtest.h>

namespace
{

TemplateExercise benchTemplate()
{
    TemplateExercise exercise(42);
    exercise.addSet(SetPrescription(5, 60.0));
    exercise.addSet(SetPrescription(5, 70.0));
    exercise.addSet(SetPrescription(5, 80.0));
    return exercise;
}

WorkoutTemplate pushTemplate()
{
    WorkoutTemplate workoutTemplate("Push A");
    workoutTemplate.addExercise(benchTemplate());
    return workoutTemplate;
}

}

class SetPrescriptionTest : public ::testing::Test
{
};

TEST_F(SetPrescriptionTest, APrescriptionHasNoCompletionOfItsOwn)
{
    const Set set = SetPrescription(5, 60.0).toSet();

    EXPECT_FALSE(set.completed());
    EXPECT_EQ(set.id(), -1);
    EXPECT_EQ(set.exerciseId(), -1);
}

TEST_F(SetPrescriptionTest, RepsAndWeightSurviveTheConversion)
{
    const Set set = SetPrescription(8, 42.5).toSet();

    EXPECT_EQ(set.repetitions(), 8);
    EXPECT_EQ(set.weight(), 42.5);
    EXPECT_EQ(set.metric(), SetMetric::Reps);
    EXPECT_EQ(set.loadType(), LoadType::External);
}

TEST_F(SetPrescriptionTest, DurationAndDistanceSurviveTheConversion)
{
    const Set timed = SetPrescription::createDuration(90).toSet();
    EXPECT_EQ(timed.metric(), SetMetric::Duration);
    EXPECT_EQ(timed.durationSeconds(), 90);
    EXPECT_EQ(timed.loadType(), LoadType::None);

    const Set covered = SetPrescription::createDistance(5000.0, 1500).toSet();
    EXPECT_EQ(covered.metric(), SetMetric::Distance);
    EXPECT_EQ(covered.distanceMeters(), 5000.0);
    EXPECT_EQ(covered.durationSeconds(), 1500);
}

TEST_F(SetPrescriptionTest, RestOverrideCarriesThrough)
{
    SetPrescription prescription(5, 60.0);
    prescription.setRestSecondsOverride(45);

    EXPECT_EQ(prescription.toSet().effectiveRestSeconds(120), 45);
}

TEST_F(SetPrescriptionTest, APerformedSetCanBeTurnedBackIntoAPrescription)
{
    Set performed(5, 82.5);
    performed.setCompleted(true);
    performed.setId(9);
    performed.setRestSecondsOverride(30);

    const SetPrescription prescription = SetPrescription::fromSet(performed);

    EXPECT_EQ(prescription.repetitions(), 5);
    EXPECT_EQ(prescription.weight(), 82.5);
    EXPECT_EQ(prescription.restSecondsOverride(), 30);
    EXPECT_FALSE(prescription.toSet().completed());
}

class TemplateExerciseTest : public ::testing::Test
{
};

TEST_F(TemplateExerciseTest, PrescriptionsAreOrdered)
{
    TemplateExercise exercise = benchTemplate();

    exercise.moveSet(2, 0);

    EXPECT_EQ(exercise.sets()[0].weight(), 80.0);
    for (size_t i = 0; i < exercise.sets().size(); ++i)
        EXPECT_EQ(exercise.sets()[i].position(), static_cast<int>(i));
}

TEST_F(TemplateExerciseTest, RestFallsBackToTheDefinitionDefault)
{
    TemplateExercise exercise = benchTemplate();
    EXPECT_EQ(exercise.effectiveRestSeconds(120), 120);

    exercise.setRestSecondsOverride(180);
    EXPECT_EQ(exercise.effectiveRestSeconds(120), 180);
}

TEST_F(TemplateExerciseTest, ATemplateExerciseMustReferenceADefinition)
{
    TemplateExercise adHoc;
    adHoc.addSet(SetPrescription(5, 60.0));

    EXPECT_FALSE(adHoc.isValid());
    EXPECT_TRUE(benchTemplate().isValid());
}

TEST_F(TemplateExerciseTest, ATemplateExerciseWithoutSetsIsReported)
{
    TemplateExercise empty(42);

    EXPECT_FALSE(empty.isValid());
}

class ToExerciseTest : public ::testing::Test
{
};

TEST_F(ToExerciseTest, TheNameIsSuppliedByTheCallerNotStoredInTheTemplate)
{
    const Exercise exercise
        = benchTemplate().toExercise("Flat Barbell Bench Press", ExerciseKind::Strength, 120);

    EXPECT_EQ(exercise.name(), "Flat Barbell Bench Press");
    EXPECT_EQ(exercise.kind(), ExerciseKind::Strength);
    EXPECT_TRUE(exercise.hasDefinition());
    EXPECT_EQ(exercise.definitionId().value(), 42);
}

TEST_F(ToExerciseTest, TheSameTemplateFollowsARenamedDefinition)
{
    const TemplateExercise exercise = benchTemplate();

    const Exercise before = exercise.toExercise("Bench Press", ExerciseKind::Strength, 120);
    const Exercise after
        = exercise.toExercise("Flat Barbell Bench Press", ExerciseKind::Strength, 120);

    EXPECT_EQ(before.name(), "Bench Press");
    EXPECT_EQ(after.name(), "Flat Barbell Bench Press");
    EXPECT_EQ(before.definitionId().value(), after.definitionId().value());
}

TEST_F(ToExerciseTest, PrescriptionsBecomeConcreteUncompletedSets)
{
    const Exercise exercise
        = benchTemplate().toExercise("Bench Press", ExerciseKind::Strength, 120);

    ASSERT_EQ(exercise.sets().size(), 3u);
    EXPECT_EQ(exercise.sets()[0].weight(), 60.0);
    EXPECT_EQ(exercise.sets()[2].weight(), 80.0);
    for (const auto& set : exercise.sets())
        EXPECT_FALSE(set.completed());
    EXPECT_FALSE(exercise.isCompleted());
}

TEST_F(ToExerciseTest, RestOverrideBeatsTheDefinitionDefault)
{
    TemplateExercise exercise = benchTemplate();
    exercise.setRestSecondsOverride(180);

    EXPECT_EQ(exercise.toExercise("Bench Press", ExerciseKind::Strength, 120).restSeconds(), 180);
    EXPECT_EQ(benchTemplate().toExercise("Bench Press", ExerciseKind::Strength, 90).restSeconds(),
              90);
}

TEST_F(ToExerciseTest, NotesTravelFromTheTemplateToTheExercise)
{
    TemplateExercise exercise = benchTemplate();
    exercise.setNotes("Pause on the chest");

    EXPECT_EQ(exercise.toExercise("Bench Press", ExerciseKind::Strength, 120).notes(),
              "Pause on the chest");
}

class WorkoutTemplateTest : public ::testing::Test
{
};

TEST_F(WorkoutTemplateTest, ExercisesAreOrderedAndRenumbered)
{
    WorkoutTemplate workoutTemplate = pushTemplate();
    TemplateExercise dip(43);
    dip.addSet(SetPrescription(10, 0.0));
    workoutTemplate.addExercise(dip);

    workoutTemplate.moveExercise(1, 0);

    EXPECT_EQ(workoutTemplate.exercises()[0].definitionId(), 43);
    for (size_t i = 0; i < workoutTemplate.exercises().size(); ++i)
        EXPECT_EQ(workoutTemplate.exercises()[i].position(), static_cast<int>(i));
}

TEST_F(WorkoutTemplateTest, ATemplateKnowsWhichDefinitionsItNeeds)
{
    WorkoutTemplate workoutTemplate = pushTemplate();
    TemplateExercise dip(43);
    dip.addSet(SetPrescription(10, 0.0));
    workoutTemplate.addExercise(dip);
    workoutTemplate.addExercise(benchTemplate());

    const std::vector<int> ids = workoutTemplate.definitionIds();

    EXPECT_EQ(ids.size(), 2u);
    EXPECT_TRUE(workoutTemplate.references(42));
    EXPECT_TRUE(workoutTemplate.references(43));
    EXPECT_FALSE(workoutTemplate.references(99));
}

TEST_F(WorkoutTemplateTest, AWellFormedTemplateIsValid) { EXPECT_TRUE(pushTemplate().isValid()); }

TEST_F(WorkoutTemplateTest, NameAndContentAreRequired)
{
    WorkoutTemplate unnamed("  ");
    unnamed.addExercise(benchTemplate());
    EXPECT_FALSE(unnamed.isValid());

    WorkoutTemplate empty("Push A");
    EXPECT_TRUE(empty.isEmpty());
    EXPECT_FALSE(empty.isValid());
}

TEST_F(WorkoutTemplateTest, ErrorsFromExercisesAreReportedWithTheirIndex)
{
    WorkoutTemplate workoutTemplate = pushTemplate();
    workoutTemplate.addExercise(TemplateExercise(44));

    ASSERT_FALSE(workoutTemplate.isValid());
    EXPECT_TRUE(workoutTemplate.validationErrors().first().startsWith("exercise 1:"));
}

TEST_F(WorkoutTemplateTest, NormalizePositionsRepairsDirectVectorEdits)
{
    WorkoutTemplate workoutTemplate = pushTemplate();
    TemplateExercise dip(43);
    dip.addSet(SetPrescription(10, 0.0));
    workoutTemplate.addExercise(dip);

    std::swap(workoutTemplate.exercises()[0], workoutTemplate.exercises()[1]);
    workoutTemplate.exercises()[1].sets().push_back(SetPrescription(5, 90.0));

    workoutTemplate.normalizePositions();

    for (size_t i = 0; i < workoutTemplate.exercises().size(); ++i)
    {
        EXPECT_EQ(workoutTemplate.exercises()[i].position(), static_cast<int>(i));
        const auto& sets = workoutTemplate.exercises()[i].sets();
        for (size_t j = 0; j < sets.size(); ++j)
            EXPECT_EQ(sets[j].position(), static_cast<int>(j));
    }
}
