#include "infrastructure/exercisecatalog/exercisedefinitionrepositorydb.h"
#include "domain/exercisecatalog/exercisedefinition.h"
#include "domain/exercisecatalog/exercisedefinitionquery.h"
#include "infrastructure/exercisecatalog/muscleinvolvementserializer.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include <gtest/gtest.h>

class ExerciseDefinitionRepositoryDbTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        m_database.setDatabaseName(":memory:");
        ASSERT_TRUE(m_database.open());

        QSqlQuery pragma(m_database);
        pragma.exec("PRAGMA foreign_keys = ON;");

        m_dbStorage = std::make_unique<DbStorage>(m_database);
        m_repo = std::make_unique<ExerciseDefinitionRepositoryDb>(*m_dbStorage);
        ASSERT_TRUE(m_repo->createTables());
    }

    void TearDown() override
    {
        m_repo.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    static ExerciseDefinition benchPress()
    {
        ExerciseDefinition definition("bench-press", "Bench Press", ExerciseKind::Strength);
        definition.addAlias("Flat Bench");
        definition.addAlias("BP");
        definition.setEquipment(Equipment::Barbell);
        definition.setMechanics(Mechanics::Compound);
        definition.setLaterality(Laterality::Bilateral);
        definition.setDefaultRestSeconds(180);
        definition.setVideoUrl("https://example.com/bench");
        definition.setInstructions("Keep the shoulder blades retracted.");
        definition.addMuscle({ Muscle::Chest, MuscleRole::Primary });
        definition.addMuscle({ Muscle::Triceps, MuscleRole::Secondary });
        definition.addMuscle({ Muscle::FrontDelts, MuscleRole::Stabilizer });
        return definition;
    }

    static ExerciseDefinition squat()
    {
        ExerciseDefinition definition("back-squat", "Back Squat", ExerciseKind::Strength);
        definition.setEquipment(Equipment::Barbell);
        definition.addMuscle({ Muscle::Quads, MuscleRole::Primary });
        definition.addMuscle({ Muscle::Glutes, MuscleRole::Secondary });
        return definition;
    }

    static ExerciseDefinition running()
    {
        ExerciseDefinition definition("running", "Running", ExerciseKind::Cardio);
        definition.setEquipment(Equipment::Treadmill);
        definition.setDefaultMetric(SetMetric::Distance);
        definition.setDefaultLoadType(LoadType::None);
        return definition;
    }

    ExerciseDefinition reload(int id) const
    {
        return m_repo->findOne(ExerciseDefinitionQuery().whereId(id))
            .value_or(ExerciseDefinition());
    }

    int muscleRowCount() const
    {
        QSqlQuery query(m_database);
        query.exec(
            QStringLiteral("SELECT COUNT(*) FROM %1").arg(MuscleInvolvementSerializer::table));
        return query.next() ? query.value(0).toInt() : -1;
    }

    QString m_connectionName = "exercise_definition_repository_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<ExerciseDefinitionRepositoryDb> m_repo;
};

TEST_F(ExerciseDefinitionRepositoryDbTest, SavingAssignsAnId)
{
    const int id = m_repo->save(benchPress());

    EXPECT_GT(id, 0);
}

TEST_F(ExerciseDefinitionRepositoryDbTest, ScalarFieldsSurviveARoundTrip)
{
    const int id = m_repo->save(benchPress());

    const ExerciseDefinition loaded = reload(id);

    EXPECT_EQ(loaded.id(), id);
    EXPECT_EQ(loaded.slug(), "bench-press");
    EXPECT_EQ(loaded.name(), "Bench Press");
    EXPECT_EQ(loaded.kind(), ExerciseKind::Strength);
    EXPECT_EQ(loaded.equipment(), Equipment::Barbell);
    EXPECT_EQ(loaded.mechanics(), Mechanics::Compound);
    EXPECT_EQ(loaded.laterality(), Laterality::Bilateral);
    EXPECT_EQ(loaded.defaultRestSeconds(), 180);
    EXPECT_EQ(loaded.videoUrl(), "https://example.com/bench");
    EXPECT_EQ(loaded.instructions(), "Keep the shoulder blades retracted.");
    EXPECT_EQ(loaded.origin(), CatalogOrigin::BuiltIn);
    EXPECT_FALSE(loaded.isArchived());
}

TEST_F(ExerciseDefinitionRepositoryDbTest, MetricAndLoadTypeSurviveARoundTrip)
{
    const int id = m_repo->save(running());

    const ExerciseDefinition loaded = reload(id);

    EXPECT_EQ(loaded.kind(), ExerciseKind::Cardio);
    EXPECT_EQ(loaded.defaultMetric(), SetMetric::Distance);
    EXPECT_EQ(loaded.defaultLoadType(), LoadType::None);
}

TEST_F(ExerciseDefinitionRepositoryDbTest, AliasesSurviveARoundTripInOrder)
{
    const int id = m_repo->save(benchPress());

    const ExerciseDefinition loaded = reload(id);

    ASSERT_EQ(loaded.aliases().size(), 2);
    EXPECT_EQ(loaded.aliases()[0], "Flat Bench");
    EXPECT_EQ(loaded.aliases()[1], "BP");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, ADefinitionWithoutAliasesLoadsAnEmptyList)
{
    const int id = m_repo->save(squat());

    EXPECT_TRUE(reload(id).aliases().isEmpty());
}

TEST_F(ExerciseDefinitionRepositoryDbTest, MusclesAndRolesSurviveARoundTripInOrder)
{
    const int id = m_repo->save(benchPress());

    const ExerciseDefinition loaded = reload(id);

    ASSERT_EQ(loaded.muscles().size(), 3u);
    EXPECT_EQ(loaded.muscles()[0].muscle, Muscle::Chest);
    EXPECT_EQ(loaded.muscles()[0].role, MuscleRole::Primary);
    EXPECT_EQ(loaded.muscles()[1].muscle, Muscle::Triceps);
    EXPECT_EQ(loaded.muscles()[1].role, MuscleRole::Secondary);
    EXPECT_EQ(loaded.muscles()[2].muscle, Muscle::FrontDelts);
    EXPECT_EQ(loaded.muscles()[2].role, MuscleRole::Stabilizer);
}

TEST_F(ExerciseDefinitionRepositoryDbTest, SavingTheSameSlugUpdatesInsteadOfDuplicating)
{
    const int first = m_repo->save(benchPress());

    ExerciseDefinition renamed = benchPress();
    renamed.setName("Barbell Bench Press");
    const int second = m_repo->save(renamed);

    EXPECT_EQ(second, first);
    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 1);
    EXPECT_EQ(reload(first).name(), "Barbell Bench Press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, SavingAnExistingIdUpdatesInPlace)
{
    const int id = m_repo->save(benchPress());

    ExerciseDefinition loaded = reload(id);
    loaded.setArchived(true);
    m_repo->save(loaded);

    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 1);
    EXPECT_TRUE(reload(id).isArchived());
}

TEST_F(ExerciseDefinitionRepositoryDbTest, ResavingReplacesTheMusclesInsteadOfAppending)
{
    const int id = m_repo->save(benchPress());

    ExerciseDefinition trimmed("bench-press", "Bench Press", ExerciseKind::Strength);
    trimmed.addMuscle({ Muscle::Chest, MuscleRole::Primary });
    m_repo->save(trimmed);

    EXPECT_EQ(muscleRowCount(), 1);
    EXPECT_EQ(reload(id).muscles().size(), 1u);
}

TEST_F(ExerciseDefinitionRepositoryDbTest, FindOneBySlugReturnsTheDefinition)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    const auto found = m_repo->findOne(ExerciseDefinitionQuery().whereSlug("back-squat"));

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name(), "Back Squat");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, FindOneReturnsNulloptWhenNothingMatches)
{
    m_repo->save(benchPress());

    EXPECT_FALSE(m_repo->findOne(ExerciseDefinitionQuery().whereSlug("deadlift")).has_value());
}

TEST_F(ExerciseDefinitionRepositoryDbTest, FindAllLoadsMusclesForEveryRow)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    const auto all
        = m_repo->findAll(ExerciseDefinitionQuery().orderByName(SortDirection::Ascending));

    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].muscles().size(), 2u);
    EXPECT_EQ(all[1].muscles().size(), 3u);
}

TEST_F(ExerciseDefinitionRepositoryDbTest, WhereKindFilters)
{
    m_repo->save(benchPress());
    m_repo->save(running());

    const auto cardio = m_repo->findAll(ExerciseDefinitionQuery().whereKind(ExerciseKind::Cardio));

    ASSERT_EQ(cardio.size(), 1u);
    EXPECT_EQ(cardio[0].slug(), "running");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, WhereEquipmentFilters)
{
    m_repo->save(benchPress());
    m_repo->save(running());

    const auto barbell
        = m_repo->findAll(ExerciseDefinitionQuery().whereEquipment(Equipment::Barbell));

    ASSERT_EQ(barbell.size(), 1u);
    EXPECT_EQ(barbell[0].slug(), "bench-press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, WhereOriginFilters)
{
    m_repo->save(benchPress());

    ExerciseDefinition custom("my-thing", "My Thing", ExerciseKind::Strength);
    custom.setOrigin(CatalogOrigin::Custom);
    m_repo->save(custom);

    const auto builtIn
        = m_repo->findAll(ExerciseDefinitionQuery().whereOrigin(CatalogOrigin::BuiltIn));

    ASSERT_EQ(builtIn.size(), 1u);
    EXPECT_EQ(builtIn[0].slug(), "bench-press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, WhereArchivedFilters)
{
    m_repo->save(benchPress());

    ExerciseDefinition retired = squat();
    retired.setArchived(true);
    m_repo->save(retired);

    const auto active = m_repo->findAll(ExerciseDefinitionQuery().whereArchived(false));
    const auto archived = m_repo->findAll(ExerciseDefinitionQuery().whereArchived(true));

    ASSERT_EQ(active.size(), 1u);
    EXPECT_EQ(active[0].slug(), "bench-press");
    ASSERT_EQ(archived.size(), 1u);
    EXPECT_EQ(archived[0].slug(), "back-squat");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, WhereMuscleFindsDefinitionsTargetingIt)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    const auto chest = m_repo->findAll(ExerciseDefinitionQuery().whereMuscle(Muscle::Chest));

    ASSERT_EQ(chest.size(), 1u);
    EXPECT_EQ(chest[0].slug(), "bench-press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, WhereMuscleMatchesSecondaryInvolvementToo)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    const auto triceps = m_repo->findAll(ExerciseDefinitionQuery().whereMuscle(Muscle::Triceps));

    ASSERT_EQ(triceps.size(), 1u);
    EXPECT_EQ(triceps[0].slug(), "bench-press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, WhereRegionExpandsToEveryMuscleOfThatRegion)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    const auto legs = m_repo->findAll(ExerciseDefinitionQuery().whereRegion(BodyRegion::Legs));

    ASSERT_EQ(legs.size(), 1u);
    EXPECT_EQ(legs[0].slug(), "back-squat");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, WhereRegionMatchesADefinitionOnlyOnce)
{
    m_repo->save(squat());

    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery().whereRegion(BodyRegion::Legs)), 1);
}

TEST_F(ExerciseDefinitionRepositoryDbTest, NameContainsMatchesTheName)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    const auto found = m_repo->findAll(ExerciseDefinitionQuery().whereNameContains("Squat"));

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].slug(), "back-squat");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, NameContainsMatchesAnAlias)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    const auto found = m_repo->findAll(ExerciseDefinitionQuery().whereNameContains("Flat Bench"));

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].slug(), "bench-press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, NameContainsKeepsItsOrGroupWhenCombinedWithAnotherFilter)
{
    m_repo->save(benchPress());

    ExerciseDefinition benchJump("bench-jump", "Bench Jump", ExerciseKind::Bodyweight);
    benchJump.addMuscle({ Muscle::Quads, MuscleRole::Primary });
    m_repo->save(benchJump);

    const auto found = m_repo->findAll(
        ExerciseDefinitionQuery().whereNameContains("Bench").whereKind(ExerciseKind::Strength));

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].slug(), "bench-press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, OrderByNameSortsBothWays)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    const auto ascending
        = m_repo->findAll(ExerciseDefinitionQuery().orderByName(SortDirection::Ascending));
    const auto descending
        = m_repo->findAll(ExerciseDefinitionQuery().orderByName(SortDirection::Descending));

    ASSERT_EQ(ascending.size(), 2u);
    EXPECT_EQ(ascending[0].name(), "Back Squat");
    EXPECT_EQ(descending[0].name(), "Bench Press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, LimitAndOffsetPageThroughTheCatalog)
{
    m_repo->save(benchPress());
    m_repo->save(squat());
    m_repo->save(running());

    const auto page = m_repo->findAll(
        ExerciseDefinitionQuery().orderByName(SortDirection::Ascending).withLimit(1).withOffset(1));

    ASSERT_EQ(page.size(), 1u);
    EXPECT_EQ(page[0].name(), "Bench Press");
}

TEST_F(ExerciseDefinitionRepositoryDbTest, CountAndExistsAgreeWithTheFilter)
{
    m_repo->save(benchPress());
    m_repo->save(squat());

    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 2);
    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery().whereMuscle(Muscle::Chest)), 1);
    EXPECT_TRUE(m_repo->exists(ExerciseDefinitionQuery().whereSlug("back-squat")));
    EXPECT_FALSE(m_repo->exists(ExerciseDefinitionQuery().whereSlug("deadlift")));
}

TEST_F(ExerciseDefinitionRepositoryDbTest, RemoveDeletesTheDefinitionAndCascadesToItsMuscles)
{
    const int id = m_repo->save(benchPress());
    m_repo->save(squat());

    EXPECT_TRUE(m_repo->remove(ExerciseDefinitionQuery().whereId(id)));

    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 1);
    EXPECT_EQ(muscleRowCount(), 2);
}

TEST_F(ExerciseDefinitionRepositoryDbTest, RemoveReturnsFalseWhenNothingMatches)
{
    m_repo->save(benchPress());

    EXPECT_FALSE(m_repo->remove(ExerciseDefinitionQuery().whereSlug("deadlift")));
}
