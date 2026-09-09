#include "domain/exercisecatalog/exercisedefinitionquery.h"
#include <gtest/gtest.h>

class ExerciseDefinitionQueryTest : public ::testing::Test
{
};

TEST_F(ExerciseDefinitionQueryTest, AFreshQueryConstrainsNothing)
{
    ExerciseDefinitionQuery query;

    EXPECT_FALSE(query.id().has_value());
    EXPECT_FALSE(query.slug().has_value());
    EXPECT_FALSE(query.nameContains().has_value());
    EXPECT_FALSE(query.kind().has_value());
    EXPECT_FALSE(query.equipment().has_value());
    EXPECT_FALSE(query.muscle().has_value());
    EXPECT_FALSE(query.region().has_value());
    EXPECT_FALSE(query.origin().has_value());
    EXPECT_FALSE(query.archived().has_value());
    EXPECT_FALSE(query.orderByNameDirection().has_value());
    EXPECT_FALSE(query.limit().has_value());
    EXPECT_FALSE(query.offset().has_value());
}

TEST_F(ExerciseDefinitionQueryTest, FiltersChainAndAreReadBack)
{
    ExerciseDefinitionQuery query;

    query.whereKind(ExerciseKind::Strength)
        .whereEquipment(Equipment::Barbell)
        .whereMuscle(Muscle::Quads)
        .whereRegion(BodyRegion::Legs)
        .whereOrigin(CatalogOrigin::BuiltIn)
        .whereArchived(false)
        .whereNameContains("squat")
        .orderByName(SortDirection::Ascending)
        .withLimit(20)
        .withOffset(5);

    EXPECT_EQ(query.kind().value(), ExerciseKind::Strength);
    EXPECT_EQ(query.equipment().value(), Equipment::Barbell);
    EXPECT_EQ(query.muscle().value(), Muscle::Quads);
    EXPECT_EQ(query.region().value(), BodyRegion::Legs);
    EXPECT_EQ(query.origin().value(), CatalogOrigin::BuiltIn);
    EXPECT_FALSE(query.archived().value());
    EXPECT_EQ(query.nameContains().value(), "squat");
    EXPECT_EQ(query.orderByNameDirection().value(), SortDirection::Ascending);
    EXPECT_EQ(query.limit().value(), 20);
    EXPECT_EQ(query.offset().value(), 5);
}

TEST_F(ExerciseDefinitionQueryTest, IdAndSlugAreIndependentLookups)
{
    ExerciseDefinitionQuery byId;
    byId.whereId(7);

    ExerciseDefinitionQuery bySlug;
    bySlug.whereSlug("barbell-back-squat");

    EXPECT_EQ(byId.id().value(), 7);
    EXPECT_FALSE(byId.slug().has_value());
    EXPECT_EQ(bySlug.slug().value(), "barbell-back-squat");
    EXPECT_FALSE(bySlug.id().has_value());
}

TEST_F(ExerciseDefinitionQueryTest, TheLastValueForAFilterWins)
{
    ExerciseDefinitionQuery query;

    query.withLimit(10).withLimit(50);

    EXPECT_EQ(query.limit().value(), 50);
}
