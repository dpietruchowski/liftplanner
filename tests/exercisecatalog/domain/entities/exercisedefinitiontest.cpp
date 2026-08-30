#include "modules/exercisecatalog/domain/entities/exercisedefinition.h"
#include <gtest/gtest.h>

namespace
{

ExerciseDefinition makeSquat()
{
    ExerciseDefinition definition("barbell-back-squat", "Barbell Back Squat",
                                  ExerciseKind::Strength);
    definition.setEquipment(Equipment::Barbell);
    definition.addMuscle({ Muscle::Quads, MuscleRole::Primary });
    definition.addMuscle({ Muscle::Glutes, MuscleRole::Secondary });
    definition.addMuscle({ Muscle::Abs, MuscleRole::Stabilizer });
    return definition;
}

}

class ExerciseDefinitionTest : public ::testing::Test
{
};

TEST_F(ExerciseDefinitionTest, DefaultsMatchTheStrengthCase)
{
    ExerciseDefinition definition;

    EXPECT_EQ(definition.id(), -1);
    EXPECT_TRUE(definition.slug().isEmpty());
    EXPECT_EQ(definition.kind(), ExerciseKind::Strength);
    EXPECT_EQ(definition.defaultMetric(), SetMetric::Reps);
    EXPECT_EQ(definition.defaultLoadType(), LoadType::External);
    EXPECT_EQ(definition.defaultRestSeconds(), 120);
    EXPECT_EQ(definition.origin(), CatalogOrigin::BuiltIn);
    EXPECT_FALSE(definition.isArchived());
}

TEST_F(ExerciseDefinitionTest, ConstructorDerivesMetricAndLoadTypeFromKind)
{
    ExerciseDefinition run("outdoor-run", "Outdoor Run", ExerciseKind::Cardio);
    EXPECT_EQ(run.defaultMetric(), SetMetric::Distance);
    EXPECT_EQ(run.defaultLoadType(), LoadType::None);

    ExerciseDefinition pullUp("pull-up", "Pull-up", ExerciseKind::Bodyweight);
    EXPECT_EQ(pullUp.defaultMetric(), SetMetric::Reps);
    EXPECT_EQ(pullUp.defaultLoadType(), LoadType::Bodyweight);

    ExerciseDefinition plank("plank", "Plank", ExerciseKind::Mobility);
    EXPECT_EQ(plank.defaultMetric(), SetMetric::Duration);
}

class SlugifyTest : public ::testing::Test
{
};

TEST_F(SlugifyTest, TurnsADisplayNameIntoKebabCase)
{
    EXPECT_EQ(ExerciseDefinition::slugify("Barbell Back Squat"), "barbell-back-squat");
    EXPECT_EQ(ExerciseDefinition::slugify("Pull-up"), "pull-up");
    EXPECT_EQ(ExerciseDefinition::slugify("Farmer's Walk"), "farmer-s-walk");
}

TEST_F(SlugifyTest, CollapsesSeparatorsAndTrimsEdges)
{
    EXPECT_EQ(ExerciseDefinition::slugify("  Bench   Press  "), "bench-press");
    EXPECT_EQ(ExerciseDefinition::slugify("--Squat--"), "squat");
    EXPECT_EQ(ExerciseDefinition::slugify("Row / Pull"), "row-pull");
}

TEST_F(SlugifyTest, FoldsAccents)
{
    EXPECT_EQ(ExerciseDefinition::slugify("Przysiad ze sztangą"), "przysiad-ze-sztanga");
}

TEST_F(SlugifyTest, KeepsDigits)
{
    EXPECT_EQ(ExerciseDefinition::slugify("Farmer Walk 20m"), "farmer-walk-20m");
}

class ExerciseDefinitionValidationTest : public ::testing::Test
{
};

TEST_F(ExerciseDefinitionValidationTest, AFullyDescribedStrengthMovementIsValid)
{
    EXPECT_TRUE(makeSquat().isValid());
    EXPECT_TRUE(makeSquat().validationErrors().isEmpty());
}

TEST_F(ExerciseDefinitionValidationTest, SlugMustBeKebabCase)
{
    ExerciseDefinition definition = makeSquat();

    definition.setSlug("");
    EXPECT_FALSE(definition.isValid());

    definition.setSlug("Barbell Back Squat");
    EXPECT_FALSE(definition.isValid());

    definition.setSlug("barbell--squat");
    EXPECT_FALSE(definition.isValid());

    definition.setSlug("-squat");
    EXPECT_FALSE(definition.isValid());

    definition.setSlug("squat-");
    EXPECT_FALSE(definition.isValid());

    definition.setSlug("squat-2");
    EXPECT_TRUE(definition.isValid());
}

TEST_F(ExerciseDefinitionValidationTest, NameMustNotBeEmpty)
{
    ExerciseDefinition definition = makeSquat();
    definition.setName("   ");

    EXPECT_FALSE(definition.isValid());
}

TEST_F(ExerciseDefinitionValidationTest, StrengthMovementNeedsAPrimaryMuscle)
{
    ExerciseDefinition definition("bench-press", "Bench Press", ExerciseKind::Strength);
    EXPECT_FALSE(definition.isValid());

    definition.addMuscle({ Muscle::Chest, MuscleRole::Secondary });
    EXPECT_FALSE(definition.isValid());

    definition.addMuscle({ Muscle::Chest, MuscleRole::Primary });
    EXPECT_TRUE(definition.isValid());
}

TEST_F(ExerciseDefinitionValidationTest, CardioDoesNotNeedMuscles)
{
    ExerciseDefinition run("outdoor-run", "Outdoor Run", ExerciseKind::Cardio);

    EXPECT_TRUE(run.isValid());
}

TEST_F(ExerciseDefinitionValidationTest, MetricMustSuitTheKind)
{
    ExerciseDefinition definition = makeSquat();
    definition.setDefaultMetric(SetMetric::Distance);

    EXPECT_FALSE(definition.isValid());
    EXPECT_EQ(definition.validationErrors().size(), 1);

    ExerciseDefinition run("outdoor-run", "Outdoor Run", ExerciseKind::Cardio);
    run.setDefaultMetric(SetMetric::Reps);
    EXPECT_FALSE(run.isValid());
}

TEST_F(ExerciseDefinitionValidationTest, RestSecondsMustNotBeNegative)
{
    ExerciseDefinition definition = makeSquat();
    definition.setDefaultRestSeconds(-1);

    EXPECT_FALSE(definition.isValid());
}

class MuscleInvolvementOnDefinitionTest : public ::testing::Test
{
};

TEST_F(MuscleInvolvementOnDefinitionTest, MusclesAreQueryableByRole)
{
    const ExerciseDefinition squat = makeSquat();

    ASSERT_EQ(squat.musclesWithRole(MuscleRole::Primary).size(), 1u);
    EXPECT_EQ(squat.musclesWithRole(MuscleRole::Primary)[0], Muscle::Quads);
    EXPECT_EQ(squat.musclesWithRole(MuscleRole::Secondary)[0], Muscle::Glutes);
    EXPECT_EQ(squat.musclesWithRole(MuscleRole::Stabilizer)[0], Muscle::Abs);
}

TEST_F(MuscleInvolvementOnDefinitionTest, AddingTheSameMuscleTwiceUpdatesItsRole)
{
    ExerciseDefinition definition = makeSquat();

    definition.addMuscle({ Muscle::Glutes, MuscleRole::Primary });

    EXPECT_EQ(definition.muscles().size(), 3u);
    EXPECT_EQ(definition.musclesWithRole(MuscleRole::Primary).size(), 2u);
    EXPECT_TRUE(definition.musclesWithRole(MuscleRole::Secondary).empty());
}

TEST_F(MuscleInvolvementOnDefinitionTest, RegionsAreDerivedAndDeduplicated)
{
    ExerciseDefinition definition = makeSquat();
    definition.addMuscle({ Muscle::Hamstrings, MuscleRole::Secondary });

    const std::vector<BodyRegion> regions = definition.regions();

    EXPECT_EQ(regions.size(), 2u);
    EXPECT_EQ(regions[0], BodyRegion::Legs);
    EXPECT_EQ(regions[1], BodyRegion::Core);
}

TEST_F(MuscleInvolvementOnDefinitionTest, TargetsAnswersPerMuscle)
{
    const ExerciseDefinition squat = makeSquat();

    EXPECT_TRUE(squat.targets(Muscle::Quads));
    EXPECT_TRUE(squat.targets(Muscle::Abs));
    EXPECT_FALSE(squat.targets(Muscle::Biceps));
}

class ImportedDefinitionTest : public ::testing::Test
{
};

TEST_F(ImportedDefinitionTest, CreateImportedMarksTheOriginAndBuildsASlug)
{
    const ExerciseDefinition imported
        = ExerciseDefinition::createImported("Flat Barbell Bench Press", ExerciseKind::Strength);

    EXPECT_EQ(imported.origin(), CatalogOrigin::Imported);
    EXPECT_EQ(imported.slug(), "flat-barbell-bench-press");
    EXPECT_EQ(imported.name(), "Flat Barbell Bench Press");
}

TEST_F(ImportedDefinitionTest, ImportedIsDistinctFromUserCreated)
{
    ExerciseDefinition imported
        = ExerciseDefinition::createImported("Zercher Squat", ExerciseKind::Strength);
    ExerciseDefinition custom("zercher-squat", "Zercher Squat", ExerciseKind::Strength);
    custom.setOrigin(CatalogOrigin::Custom);

    EXPECT_NE(imported.origin(), custom.origin());
    EXPECT_TRUE(imported.isDeletable());
    EXPECT_TRUE(custom.isDeletable());
}

TEST_F(ImportedDefinitionTest, BuiltInIsNotDeletable) { EXPECT_FALSE(makeSquat().isDeletable()); }

class AbsorbTest : public ::testing::Test
{
};

TEST_F(AbsorbTest, TheLosersNameBecomesAnAlias)
{
    ExerciseDefinition winner = makeSquat();
    ExerciseDefinition loser
        = ExerciseDefinition::createImported("Back Squat", ExerciseKind::Strength);

    winner.absorb(loser);

    EXPECT_TRUE(winner.aliases().contains("Back Squat"));
    EXPECT_TRUE(winner.matchesName("Back Squat"));
}

TEST_F(AbsorbTest, TheLosersAliasesCarryOver)
{
    ExerciseDefinition winner = makeSquat();
    ExerciseDefinition loser
        = ExerciseDefinition::createImported("Back Squat", ExerciseKind::Strength);
    loser.addAlias("Squat");

    winner.absorb(loser);

    EXPECT_TRUE(winner.matchesName("Squat"));
    EXPECT_TRUE(winner.matchesName("Back Squat"));
}

TEST_F(AbsorbTest, AbsorbingTwiceDoesNotDuplicateAliases)
{
    ExerciseDefinition winner = makeSquat();
    ExerciseDefinition loser
        = ExerciseDefinition::createImported("Back Squat", ExerciseKind::Strength);

    winner.absorb(loser);
    winner.absorb(loser);

    EXPECT_EQ(winner.aliases().size(), 1);
}

TEST_F(AbsorbTest, AnAliasEqualToTheOwnNameIsIgnored)
{
    ExerciseDefinition definition = makeSquat();

    definition.addAlias("barbell back squat");
    definition.addAlias("  ");
    definition.addAlias("");

    EXPECT_TRUE(definition.aliases().isEmpty());
}

TEST_F(AbsorbTest, MatchingIgnoresCaseAndAccents)
{
    ExerciseDefinition definition = makeSquat();
    definition.addAlias("Przysiad ze sztangą");

    EXPECT_TRUE(definition.matchesName("BARBELL BACK SQUAT"));
    EXPECT_TRUE(definition.matchesName("przysiad ze sztanga"));
    EXPECT_FALSE(definition.matchesName("Front Squat"));
}
