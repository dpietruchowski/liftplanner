#include "modules/exercisecatalog/infrastructure/backfill/exercisehistorybackfill.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionquery.h"
#include "modules/exercisecatalog/infrastructure/database/exercisedefinitionrepositorydb.h"
#include "modules/exercisecatalog/infrastructure/seed/exercisecatalogseed.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "modules/workout/infrastructure/serializers/exerciseserializer.h"

#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>
#include <gtest/gtest.h>

class ExerciseHistoryBackfillTest : public ::testing::Test
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
        m_workoutRepo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
        ASSERT_TRUE(m_workoutRepo->createTables());

        MigrationRunner runner(*m_dbStorage);
        m_workoutRepo->registerMigrations(runner);
        ASSERT_TRUE(runner.run());

        m_catalog = std::make_unique<ExerciseDefinitionRepositoryDb>(*m_dbStorage);
        ASSERT_TRUE(m_catalog->createTables());
    }

    void TearDown() override
    {
        m_catalog.reset();
        m_workoutRepo.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    void seedShippedCatalog()
    {
        QFile file(QStringLiteral(EXERCISE_SEED_FILE));
        ASSERT_TRUE(file.open(QIODevice::ReadOnly));

        QStringList errors;
        const auto definitions = ExerciseCatalogSeed::parse(file.readAll(), errors);
        ASSERT_TRUE(errors.isEmpty());
        ExerciseCatalogSeed::sync(*m_catalog, definitions);
    }

    int saveWorkoutWith(const QStringList& exerciseNames)
    {
        Workout workout("Session", QDateTime::currentDateTime());
        for (const QString& name : exerciseNames)
        {
            Exercise exercise = Exercise::createAdHoc(name, ExerciseKind::Strength, 120);
            exercise.addSet(Set(5, 100.0));
            workout.addExercise(exercise);
        }
        return m_workoutRepo->save(workout);
    }

    HistoryBackfillResult run() { return ExerciseHistoryBackfill::apply(m_database, *m_catalog); }

    std::optional<int> definitionIdOf(const QString& name) const
    {
        QSqlQuery query(m_database);
        query.prepare(QStringLiteral("SELECT %1 FROM %2 WHERE %3 = ?")
                          .arg(ExerciseSerializer::definition_id_key, ExerciseSerializer::table,
                               ExerciseSerializer::name_key));
        query.addBindValue(name);
        if (!query.exec() || !query.next())
            return std::nullopt;

        const QVariant value = query.value(0);
        return value.isNull() ? std::nullopt : std::optional<int>(value.toInt());
    }

    int slugIdOf(const QString& slug) const
    {
        const auto found = m_catalog->findOne(ExerciseDefinitionQuery().whereSlug(slug));
        return found.has_value() ? found->id() : -1;
    }

    QString m_connectionName = "exercise_history_backfill_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<ExerciseDefinitionRepositoryDb> m_catalog;
};

TEST_F(ExerciseHistoryBackfillTest, AnExactNameIsLinkedToItsDefinition)
{
    seedShippedCatalog();
    saveWorkoutWith({ "Bench Press" });

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.examined, 1);
    EXPECT_EQ(result.linked, 1);
    EXPECT_EQ(result.unmatched, 0);
    EXPECT_EQ(definitionIdOf("Bench Press"), slugIdOf("bench-press"));
}

TEST_F(ExerciseHistoryBackfillTest, MatchingIgnoresCaseAndPunctuation)
{
    seedShippedCatalog();
    saveWorkoutWith({ "romanian deadlift" });

    run();

    EXPECT_EQ(definitionIdOf("romanian deadlift"), slugIdOf("romanian-deadlift"));
}

TEST_F(ExerciseHistoryBackfillTest, AnAliasIsLinkedToItsDefinition)
{
    seedShippedCatalog();
    saveWorkoutWith({ "RDL" });

    run();

    EXPECT_EQ(definitionIdOf("RDL"), slugIdOf("romanian-deadlift"));
}

TEST_F(ExerciseHistoryBackfillTest, ANameCarryingTwoExercisesIsLeftAlone)
{
    seedShippedCatalog();
    saveWorkoutWith({ "Pull-Up / Lat Pulldown" });

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.linked, 0);
    EXPECT_EQ(result.unmatched, 1);
    EXPECT_FALSE(definitionIdOf("Pull-Up / Lat Pulldown").has_value());
}

TEST_F(ExerciseHistoryBackfillTest, AnUnknownNameIsLeftAlone)
{
    seedShippedCatalog();
    saveWorkoutWith({ "Cable Crunch" });

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.linked, 0);
    EXPECT_EQ(result.unmatched, 1);
    EXPECT_FALSE(definitionIdOf("Cable Crunch").has_value());
}

TEST_F(ExerciseHistoryBackfillTest, AnAmbiguousNameIsNeverGuessed)
{
    ExerciseDefinition first("row-a", "Row", ExerciseKind::Strength);
    first.addMuscle({ Muscle::Lats, MuscleRole::Primary });
    ExerciseDefinition second("row-b", "row", ExerciseKind::Strength);
    second.addMuscle({ Muscle::Lats, MuscleRole::Primary });
    m_catalog->save(first);
    m_catalog->save(second);

    saveWorkoutWith({ "Row" });

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.linked, 0);
    EXPECT_EQ(result.unmatched, 1);
    EXPECT_FALSE(definitionIdOf("Row").has_value());
}

TEST_F(ExerciseHistoryBackfillTest, ArchivedDefinitionsAreNotMatched)
{
    seedShippedCatalog();

    auto plank = m_catalog->findOne(ExerciseDefinitionQuery().whereSlug("plank"));
    ASSERT_TRUE(plank.has_value());
    plank->setArchived(true);
    m_catalog->save(plank.value());

    saveWorkoutWith({ "Plank" });

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.linked, 0);
    EXPECT_FALSE(definitionIdOf("Plank").has_value());
}

TEST_F(ExerciseHistoryBackfillTest, TheSnapshotIsNotRewrittenToMatchTheCatalog)
{
    seedShippedCatalog();
    saveWorkoutWith({ "Plank" });

    run();

    QSqlQuery query(m_database);
    query.exec(QStringLiteral("SELECT %1 FROM %2 WHERE %3 = 'Plank'")
                   .arg(ExerciseSerializer::kind_key, ExerciseSerializer::table,
                        ExerciseSerializer::name_key));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), "strength");
    EXPECT_TRUE(definitionIdOf("Plank").has_value());
}

TEST_F(ExerciseHistoryBackfillTest, AlreadyLinkedRowsAreNotExamined)
{
    seedShippedCatalog();
    saveWorkoutWith({ "Bench Press", "Back Squat" });

    const HistoryBackfillResult first = run();
    const HistoryBackfillResult second = run();

    EXPECT_EQ(first.examined, 2);
    EXPECT_EQ(first.linked, 2);
    EXPECT_EQ(second.examined, 0);
    EXPECT_EQ(second.linked, 0);
}

TEST_F(ExerciseHistoryBackfillTest, AnUnmatchedRowIsRetriedOnTheNextRun)
{
    seedShippedCatalog();
    saveWorkoutWith({ "Zercher Squat" });

    EXPECT_EQ(run().unmatched, 1);

    ExerciseDefinition added("zercher-squat", "Zercher Squat", ExerciseKind::Strength);
    added.addMuscle({ Muscle::Quads, MuscleRole::Primary });
    m_catalog->save(added);

    const HistoryBackfillResult second = run();

    EXPECT_EQ(second.examined, 1);
    EXPECT_EQ(second.linked, 1);
    EXPECT_EQ(definitionIdOf("Zercher Squat"), slugIdOf("zercher-squat"));
}

TEST_F(ExerciseHistoryBackfillTest, RepeatedNamesAllResolveToTheSameDefinition)
{
    seedShippedCatalog();
    saveWorkoutWith({ "Bench Press" });
    saveWorkoutWith({ "Bench Press" });

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.examined, 2);
    EXPECT_EQ(result.linked, 2);

    QSqlQuery query(m_database);
    query.exec(QStringLiteral("SELECT COUNT(DISTINCT %1) FROM %2 WHERE %3 = 'Bench Press'")
                   .arg(ExerciseSerializer::definition_id_key, ExerciseSerializer::table,
                        ExerciseSerializer::name_key));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

TEST_F(ExerciseHistoryBackfillTest, AnEmptyHistoryIsANoOp)
{
    seedShippedCatalog();

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.examined, 0);
    EXPECT_EQ(result.linked, 0);
    EXPECT_EQ(result.unmatched, 0);
}

TEST_F(ExerciseHistoryBackfillTest, AnEmptyCatalogLeavesTheHistoryUntouched)
{
    saveWorkoutWith({ "Bench Press", "Back Squat" });

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.examined, 2);
    EXPECT_EQ(result.linked, 0);
    EXPECT_EQ(result.unmatched, 2);
    EXPECT_FALSE(definitionIdOf("Bench Press").has_value());
}

TEST_F(ExerciseHistoryBackfillTest, TheRealHistoryLinksWhatItCanAndSkipsTheRest)
{
    seedShippedCatalog();
    saveWorkoutWith(
        { "Back Squat", "Bench Press", "Pull-Up / Lat Pulldown", "Romanian Deadlift", "Plank" });
    saveWorkoutWith({ "Front Squat", "Overhead Press", "Pull-Up / Lat Pulldown", "Lateral Raise",
                      "Cable Crunch" });

    const HistoryBackfillResult result = run();

    EXPECT_EQ(result.examined, 10);
    EXPECT_EQ(result.linked, 7);
    EXPECT_EQ(result.unmatched, 3);
}
