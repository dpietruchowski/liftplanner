#include "modules/exercisecatalog/infrastructure/seed/exercisecatalogseed.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionquery.h"
#include "modules/exercisecatalog/infrastructure/database/exercisedefinitionrepositorydb.h"

#include <QFile>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include <gtest/gtest.h>

namespace
{

QByteArray seedOf(const QByteArray& entries)
{
    return QByteArray("{\"version\": 1, \"exercises\": [") + entries + QByteArray("]}");
}

const QByteArray bench_press = R"({
    "slug": "bench-press",
    "name": "Bench Press",
    "aliases": ["Flat Bench"],
    "kind": "strength",
    "equipment": "barbell",
    "mechanics": "compound",
    "laterality": "bilateral",
    "defaultMetric": "reps",
    "defaultLoadType": "external",
    "defaultRestSeconds": 180,
    "muscles": [
        { "muscle": "chest", "role": "primary" },
        { "muscle": "triceps", "role": "secondary" }
    ],
    "instructions": "Press the bar."
})";

const QByteArray squat = R"({
    "slug": "back-squat",
    "name": "Back Squat",
    "kind": "strength",
    "equipment": "barbell",
    "mechanics": "compound",
    "laterality": "bilateral",
    "defaultMetric": "reps",
    "defaultLoadType": "external",
    "defaultRestSeconds": 210,
    "muscles": [{ "muscle": "quads", "role": "primary" }]
})";

}

TEST(ExerciseCatalogSeedParseTest, AWellFormedEntryBecomesADefinition)
{
    QStringList errors;

    const auto definitions = ExerciseCatalogSeed::parse(seedOf(bench_press), errors);

    ASSERT_TRUE(errors.isEmpty()) << errors.join(", ").toStdString();
    ASSERT_EQ(definitions.size(), 1u);

    const ExerciseDefinition& definition = definitions.front();
    EXPECT_EQ(definition.slug(), "bench-press");
    EXPECT_EQ(definition.name(), "Bench Press");
    EXPECT_EQ(definition.kind(), ExerciseKind::Strength);
    EXPECT_EQ(definition.equipment(), Equipment::Barbell);
    EXPECT_EQ(definition.mechanics(), Mechanics::Compound);
    EXPECT_EQ(definition.defaultRestSeconds(), 180);
    EXPECT_EQ(definition.instructions(), "Press the bar.");
    ASSERT_EQ(definition.aliases().size(), 1);
    EXPECT_EQ(definition.aliases().first(), "Flat Bench");
    ASSERT_EQ(definition.muscles().size(), 2u);
    EXPECT_EQ(definition.muscles()[0].muscle, Muscle::Chest);
    EXPECT_EQ(definition.muscles()[1].role, MuscleRole::Secondary);
}

TEST(ExerciseCatalogSeedParseTest, EverySeededDefinitionIsMarkedBuiltIn)
{
    QStringList errors;

    const auto definitions = ExerciseCatalogSeed::parse(seedOf(bench_press), errors);

    ASSERT_EQ(definitions.size(), 1u);
    EXPECT_EQ(definitions.front().origin(), CatalogOrigin::BuiltIn);
}

TEST(ExerciseCatalogSeedParseTest, MalformedJsonIsReportedAndYieldsNothing)
{
    QStringList errors;

    const auto definitions = ExerciseCatalogSeed::parse("{ not json", errors);

    EXPECT_TRUE(definitions.empty());
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains("not valid json"));
}

TEST(ExerciseCatalogSeedParseTest, AnEmptySeedIsReported)
{
    QStringList errors;

    const auto definitions = ExerciseCatalogSeed::parse(seedOf(""), errors);

    EXPECT_TRUE(definitions.empty());
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains("no exercises"));
}

TEST(ExerciseCatalogSeedParseTest, AnInvalidEntryIsSkippedAndTheRestSurvive)
{
    const QByteArray withoutPrimaryMuscle = R"({
        "slug": "mystery-lift",
        "name": "Mystery Lift",
        "kind": "strength",
        "defaultMetric": "reps",
        "muscles": []
    })";

    QStringList errors;
    const auto definitions
        = ExerciseCatalogSeed::parse(seedOf(withoutPrimaryMuscle + "," + bench_press), errors);

    ASSERT_EQ(definitions.size(), 1u);
    EXPECT_EQ(definitions.front().slug(), "bench-press");
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains("mystery-lift"));
}

TEST(ExerciseCatalogSeedParseTest, AMetricThatDoesNotSuitTheKindIsRejected)
{
    const QByteArray timedStrength = R"({
        "slug": "timed-press",
        "name": "Timed Press",
        "kind": "strength",
        "defaultMetric": "duration",
        "muscles": [{ "muscle": "chest", "role": "primary" }]
    })";

    QStringList errors;
    const auto definitions = ExerciseCatalogSeed::parse(seedOf(timedStrength), errors);

    EXPECT_TRUE(definitions.empty());
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains("timed-press"));
}

TEST(ExerciseCatalogSeedParseTest, ADuplicatedSlugIsKeptOnlyOnce)
{
    QStringList errors;

    const auto definitions
        = ExerciseCatalogSeed::parse(seedOf(bench_press + "," + bench_press), errors);

    ASSERT_EQ(definitions.size(), 1u);
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains("duplicated slug"));
}

class ExerciseCatalogSeedSyncTest : public ::testing::Test
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

    CatalogSeedResult seed(const QByteArray& entries)
    {
        QStringList errors;
        const auto definitions = ExerciseCatalogSeed::parse(seedOf(entries), errors);
        EXPECT_TRUE(errors.isEmpty()) << errors.join(", ").toStdString();
        return ExerciseCatalogSeed::sync(*m_repo, definitions);
    }

    QString m_connectionName = "exercise_catalog_seed_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<ExerciseDefinitionRepositoryDb> m_repo;
};

TEST_F(ExerciseCatalogSeedSyncTest, TheFirstRunInsertsEverything)
{
    const CatalogSeedResult result = seed(bench_press + "," + squat);

    EXPECT_EQ(result.inserted, 2);
    EXPECT_EQ(result.updated, 0);
    EXPECT_EQ(result.archived, 0);
    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 2);
}

TEST_F(ExerciseCatalogSeedSyncTest, RunningTwiceChangesNothing)
{
    seed(bench_press + "," + squat);
    const CatalogSeedResult second = seed(bench_press + "," + squat);

    EXPECT_EQ(second.inserted, 0);
    EXPECT_EQ(second.updated, 2);
    EXPECT_EQ(second.archived, 0);
    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 2);
}

TEST_F(ExerciseCatalogSeedSyncTest, AChangedNameReachesTheExistingRow)
{
    seed(bench_press);

    QByteArray renamed = bench_press;
    renamed.replace("\"Bench Press\"", "\"Barbell Bench Press\"");
    seed(renamed);

    const auto found = m_repo->findOne(ExerciseDefinitionQuery().whereSlug("bench-press"));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name(), "Barbell Bench Press");
    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 1);
}

TEST_F(ExerciseCatalogSeedSyncTest, ABuiltInDroppedFromTheSeedIsArchivedNotDeleted)
{
    seed(bench_press + "," + squat);

    const CatalogSeedResult result = seed(bench_press);

    EXPECT_EQ(result.archived, 1);
    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 2);

    const auto found = m_repo->findOne(ExerciseDefinitionQuery().whereSlug("back-squat"));
    ASSERT_TRUE(found.has_value());
    EXPECT_TRUE(found->isArchived());
}

TEST_F(ExerciseCatalogSeedSyncTest, ABuiltInThatComesBackIsUnarchived)
{
    seed(bench_press + "," + squat);
    seed(bench_press);

    seed(bench_press + "," + squat);

    const auto found = m_repo->findOne(ExerciseDefinitionQuery().whereSlug("back-squat"));
    ASSERT_TRUE(found.has_value());
    EXPECT_FALSE(found->isArchived());
}

TEST_F(ExerciseCatalogSeedSyncTest, CustomDefinitionsAreNeverTouched)
{
    ExerciseDefinition custom("my-lift", "My Lift", ExerciseKind::Strength);
    custom.setOrigin(CatalogOrigin::Custom);
    custom.addMuscle({ Muscle::Chest, MuscleRole::Primary });
    m_repo->save(custom);

    seed(bench_press);

    const auto found = m_repo->findOne(ExerciseDefinitionQuery().whereSlug("my-lift"));
    ASSERT_TRUE(found.has_value());
    EXPECT_FALSE(found->isArchived());
    EXPECT_EQ(found->origin(), CatalogOrigin::Custom);
}

TEST_F(ExerciseCatalogSeedSyncTest, ImportedDefinitionsAreNeverArchived)
{
    ExerciseDefinition imported
        = ExerciseDefinition::createImported("Landmine Press", ExerciseKind::Strength);
    imported.addMuscle({ Muscle::FrontDelts, MuscleRole::Primary });
    m_repo->save(imported);

    seed(bench_press);

    const auto found
        = m_repo->findOne(ExerciseDefinitionQuery().whereOrigin(CatalogOrigin::Imported));
    ASSERT_TRUE(found.has_value());
    EXPECT_FALSE(found->isArchived());
}

TEST_F(ExerciseCatalogSeedSyncTest, AMissingSeedFileIsReportedWithoutTouchingTheCatalog)
{
    seed(bench_press);

    const CatalogSeedResult result = ExerciseCatalogSeed::apply(*m_repo, "/nonexistent/seed.json");

    ASSERT_EQ(result.errors.size(), 1);
    EXPECT_TRUE(result.errors.first().contains("cannot read seed"));
    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), 1);
    EXPECT_FALSE(m_repo->findOne(ExerciseDefinitionQuery().whereSlug("bench-press"))->isArchived());
}

class ShippedExerciseCatalogTest : public ExerciseCatalogSeedSyncTest
{
protected:
    static QByteArray shippedSeed()
    {
        QFile file(QStringLiteral(EXERCISE_SEED_FILE));
        if (!file.open(QIODevice::ReadOnly))
            return {};
        return file.readAll();
    }
};

TEST_F(ShippedExerciseCatalogTest, TheShippedSeedParsesWithoutErrors)
{
    QStringList errors;

    const auto definitions = ExerciseCatalogSeed::parse(shippedSeed(), errors);

    EXPECT_TRUE(errors.isEmpty()) << errors.join("\n").toStdString();
    EXPECT_GE(definitions.size(), 80u);
}

TEST_F(ShippedExerciseCatalogTest, TheShippedSeedCoversEveryBodyRegion)
{
    QStringList errors;
    const auto definitions = ExerciseCatalogSeed::parse(shippedSeed(), errors);

    QSet<int> regions;
    for (const ExerciseDefinition& definition : definitions)
    {
        for (BodyRegion region : definition.regions())
            regions.insert(static_cast<int>(region));
    }

    EXPECT_EQ(regions.size(), 7);
}

TEST_F(ShippedExerciseCatalogTest, TheShippedSeedCoversEveryEquipmentFilterItClaims)
{
    QStringList errors;
    const auto definitions = ExerciseCatalogSeed::parse(shippedSeed(), errors);

    QSet<int> kinds;
    for (const ExerciseDefinition& definition : definitions)
        kinds.insert(static_cast<int>(definition.kind()));

    EXPECT_EQ(kinds.size(), 5);
}

TEST_F(ShippedExerciseCatalogTest, TheShippedSeedLandsInTheDatabaseAndIsIdempotent)
{
    QStringList errors;
    const auto definitions = ExerciseCatalogSeed::parse(shippedSeed(), errors);
    ASSERT_TRUE(errors.isEmpty());

    const CatalogSeedResult first = ExerciseCatalogSeed::sync(*m_repo, definitions);
    const CatalogSeedResult second = ExerciseCatalogSeed::sync(*m_repo, definitions);

    EXPECT_EQ(first.inserted, static_cast<int>(definitions.size()));
    EXPECT_EQ(second.inserted, 0);
    EXPECT_EQ(second.archived, 0);
    EXPECT_EQ(m_repo->count(ExerciseDefinitionQuery()), static_cast<int>(definitions.size()));
}

TEST_F(ShippedExerciseCatalogTest, TheShippedSeedIsSearchableByMuscleAndRegion)
{
    QStringList errors;
    ExerciseCatalogSeed::sync(*m_repo, ExerciseCatalogSeed::parse(shippedSeed(), errors));

    EXPECT_GT(m_repo->count(ExerciseDefinitionQuery().whereMuscle(Muscle::Chest)), 0);
    EXPECT_GT(m_repo->count(ExerciseDefinitionQuery().whereRegion(BodyRegion::Legs)), 0);
    EXPECT_GT(m_repo->count(ExerciseDefinitionQuery().whereKind(ExerciseKind::Cardio)), 0);
    EXPECT_GT(m_repo->count(ExerciseDefinitionQuery().whereEquipment(Equipment::Barbell)), 0);
}
