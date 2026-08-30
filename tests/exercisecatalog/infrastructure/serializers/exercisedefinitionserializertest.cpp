#include "modules/exercisecatalog/infrastructure/serializers/exercisedefinitionserializer.h"
#include "modules/exercisecatalog/domain/entities/exercisedefinition.h"
#include "modules/exercisecatalog/infrastructure/serializers/muscleinvolvementserializer.h"

#include <gtest/gtest.h>

TEST(ExerciseDefinitionSerializerTest, ANewDefinitionIsSerializedWithoutAnId)
{
    ExerciseDefinition definition("pull-up", "Pull Up", ExerciseKind::Bodyweight);

    const QVariantMap data = ExerciseDefinitionSerializer::toVariant(definition);

    EXPECT_FALSE(data.contains(ExerciseDefinitionSerializer::id_key));
    EXPECT_EQ(data.value(ExerciseDefinitionSerializer::slug_key).toString(), "pull-up");
}

TEST(ExerciseDefinitionSerializerTest, EnumsAreStoredAsStableKeys)
{
    ExerciseDefinition definition("rowing", "Rowing", ExerciseKind::Cardio);
    definition.setEquipment(Equipment::Rower);
    definition.setMechanics(Mechanics::Compound);
    definition.setLaterality(Laterality::Bilateral);
    definition.setDefaultMetric(SetMetric::Distance);
    definition.setDefaultLoadType(LoadType::None);
    definition.setOrigin(CatalogOrigin::Imported);

    const QVariantMap data = ExerciseDefinitionSerializer::toVariant(definition);

    EXPECT_EQ(data.value(ExerciseDefinitionSerializer::kind_key).toString(), "cardio");
    EXPECT_EQ(data.value(ExerciseDefinitionSerializer::equipment_key).toString(), "rower");
    EXPECT_EQ(data.value(ExerciseDefinitionSerializer::default_metric_key).toString(), "distance");
    EXPECT_EQ(data.value(ExerciseDefinitionSerializer::default_load_type_key).toString(), "none");
    EXPECT_EQ(data.value(ExerciseDefinitionSerializer::origin_key).toString(), "imported");
}

TEST(ExerciseDefinitionSerializerTest, AliasesAreJoinedAndSplitBack)
{
    ExerciseDefinition definition("bench-press", "Bench Press", ExerciseKind::Strength);
    definition.addAlias("Flat Bench");
    definition.addAlias("BP");

    const QVariantMap data = ExerciseDefinitionSerializer::toVariant(definition);
    const ExerciseDefinition restored = ExerciseDefinitionSerializer::fromVariant(data);

    EXPECT_EQ(data.value(ExerciseDefinitionSerializer::aliases_key).toString(), "Flat Bench\nBP");
    EXPECT_EQ(restored.aliases(), definition.aliases());
}

TEST(ExerciseDefinitionSerializerTest, AnEmptyAliasColumnRestoresAnEmptyList)
{
    QVariantMap data;
    data.insert(ExerciseDefinitionSerializer::slug_key, "squat");
    data.insert(ExerciseDefinitionSerializer::name_key, "Squat");
    data.insert(ExerciseDefinitionSerializer::aliases_key, QString());

    EXPECT_TRUE(ExerciseDefinitionSerializer::fromVariant(data).aliases().isEmpty());
}

TEST(ExerciseDefinitionSerializerTest, ArchivedIsStoredAsAnInteger)
{
    ExerciseDefinition definition("squat", "Squat", ExerciseKind::Strength);
    definition.setArchived(true);

    const QVariantMap data = ExerciseDefinitionSerializer::toVariant(definition);

    EXPECT_EQ(data.value(ExerciseDefinitionSerializer::archived_key).toInt(), 1);
    EXPECT_TRUE(ExerciseDefinitionSerializer::fromVariant(data).isArchived());
}

TEST(ExerciseDefinitionSerializerTest, UnknownEnumKeysFallBackToTheDefault)
{
    QVariantMap data;
    data.insert(ExerciseDefinitionSerializer::kind_key, "nonsense");
    data.insert(ExerciseDefinitionSerializer::equipment_key, "nonsense");
    data.insert(ExerciseDefinitionSerializer::origin_key, "nonsense");

    const ExerciseDefinition restored = ExerciseDefinitionSerializer::fromVariant(data);

    EXPECT_EQ(restored.kind(), ExerciseKind::Strength);
    EXPECT_EQ(restored.equipment(), Equipment::Other);
    EXPECT_EQ(restored.origin(), CatalogOrigin::BuiltIn);
}

TEST(MuscleInvolvementSerializerTest, TheJoinRowCarriesItsParentAndPosition)
{
    const QVariantMap data
        = MuscleInvolvementSerializer::toVariant({ Muscle::Lats, MuscleRole::Primary }, 7, 2);

    EXPECT_EQ(data.value(MuscleInvolvementSerializer::definition_id_key).toInt(), 7);
    EXPECT_EQ(data.value(MuscleInvolvementSerializer::muscle_key).toString(), "lats");
    EXPECT_EQ(data.value(MuscleInvolvementSerializer::role_key).toString(), "primary");
    EXPECT_EQ(data.value(MuscleInvolvementSerializer::position_key).toInt(), 2);
}

TEST(MuscleInvolvementSerializerTest, TheJoinRowRestoresMuscleAndRole)
{
    const QVariantMap data = MuscleInvolvementSerializer::toVariant(
        { Muscle::Hamstrings, MuscleRole::Stabilizer }, 3, 0);

    const MuscleInvolvement restored = MuscleInvolvementSerializer::fromVariant(data);

    EXPECT_EQ(restored.muscle, Muscle::Hamstrings);
    EXPECT_EQ(restored.role, MuscleRole::Stabilizer);
}
