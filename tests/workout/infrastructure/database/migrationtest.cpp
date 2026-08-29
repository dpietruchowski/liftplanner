#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "modules/workout/infrastructure/serializers/exerciseserializer.h"
#include "modules/workout/infrastructure/serializers/setserializer.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>
#include <gtest/gtest.h>

class MigrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        m_database.setDatabaseName(":memory:");
        ASSERT_TRUE(m_database.open());

        QSqlQuery pragma(m_database);
        pragma.exec("PRAGMA foreign_keys = ON;");
    }

    void TearDown() override
    {
        m_repo.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    void createLegacySchema()
    {
        QSqlQuery query(m_database);
        ASSERT_TRUE(query.exec("CREATE TABLE workouts ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                               "name TEXT, created_time TEXT, planned_time TEXT, "
                               "started_time TEXT, ended_time TEXT, status TEXT)"));
        ASSERT_TRUE(query.exec("CREATE TABLE exercises ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                               "workout_id INTEGER NOT NULL, name TEXT, description TEXT, "
                               "rest_seconds INTEGER, "
                               "FOREIGN KEY (workout_id) REFERENCES workouts(id) "
                               "ON DELETE CASCADE)"));
        ASSERT_TRUE(query.exec("CREATE TABLE sets ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                               "exercise_id INTEGER NOT NULL, repetitions INTEGER, weight REAL, "
                               "completed INTEGER DEFAULT 0, "
                               "FOREIGN KEY (exercise_id) REFERENCES exercises(id) "
                               "ON DELETE CASCADE)"));
        ASSERT_TRUE(query.exec("PRAGMA user_version = 1"));
    }

    void insertLegacyData()
    {
        QSqlQuery query(m_database);
        ASSERT_TRUE(query.exec("INSERT INTO workouts (id, name, started_time, ended_time, status) "
                               "VALUES (1, 'Push Day', '2026-05-01T18:00:00', "
                               "'2026-05-01T19:10:00', 'Ended')"));
        ASSERT_TRUE(query.exec("INSERT INTO exercises (id, workout_id, name, description, "
                               "rest_seconds) VALUES (1, 1, 'Bench Press', 'Flat bench', 120)"));
        ASSERT_TRUE(query.exec("INSERT INTO sets (id, exercise_id, repetitions, weight, completed) "
                               "VALUES (1, 1, 10, 80.0, 1)"));
        ASSERT_TRUE(query.exec("INSERT INTO sets (id, exercise_id, repetitions, weight, completed) "
                               "VALUES (2, 1, 8, 85.5, 0)"));
    }

    void migrate()
    {
        m_dbStorage = std::make_unique<DbStorage>(m_database);
        m_repo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
        ASSERT_TRUE(m_repo->createTables());

        MigrationRunner runner(*m_dbStorage);
        m_repo->registerMigrations(runner);
        ASSERT_TRUE(runner.run());
        m_schemaVersion = runner.currentVersion();
    }

    QStringList columnsOf(const QString& table) const
    {
        QStringList columns;
        QSqlQuery query(m_database);
        query.exec(QStringLiteral("PRAGMA table_info(%1)").arg(table));
        while (query.next())
            columns.append(query.value(1).toString());
        return columns;
    }

    QVariant scalar(const QString& sql) const
    {
        QSqlQuery query(m_database);
        query.exec(sql);
        return query.next() ? query.value(0) : QVariant();
    }

    QString m_connectionName = "workout_migration_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_repo;
    int m_schemaVersion { 0 };
};

TEST_F(MigrationTest, LegacyDatabase_GainsNewSetColumns)
{
    createLegacySchema();
    insertLegacyData();

    migrate();

    const QStringList columns = columnsOf(SetSerializer::table);
    EXPECT_TRUE(columns.contains(SetSerializer::metric_key));
    EXPECT_TRUE(columns.contains(SetSerializer::load_type_key));
    EXPECT_TRUE(columns.contains(SetSerializer::duration_seconds_key));
    EXPECT_TRUE(columns.contains(SetSerializer::distance_meters_key));
    EXPECT_TRUE(columns.contains(SetSerializer::rest_seconds_override_key));
}

TEST_F(MigrationTest, LegacyDatabase_GainsExerciseKindColumn)
{
    createLegacySchema();
    insertLegacyData();

    migrate();

    EXPECT_TRUE(columnsOf(ExerciseSerializer::table).contains(ExerciseSerializer::kind_key));
}

TEST_F(MigrationTest, LegacyRows_AreBackfilledWithWeightedRepsDefaults)
{
    createLegacySchema();
    insertLegacyData();

    migrate();

    EXPECT_EQ(scalar("SELECT metric FROM sets WHERE id = 1").toString(), "reps");
    EXPECT_EQ(scalar("SELECT load_type FROM sets WHERE id = 1").toString(), "external");
    EXPECT_EQ(scalar("SELECT duration_seconds FROM sets WHERE id = 1").toInt(), 0);
    EXPECT_DOUBLE_EQ(scalar("SELECT distance_meters FROM sets WHERE id = 1").toDouble(), 0.0);
    EXPECT_EQ(scalar("SELECT rest_seconds_override FROM sets WHERE id = 1").toInt(), -1);
    EXPECT_EQ(scalar("SELECT kind FROM exercises WHERE id = 1").toString(), "strength");
}

TEST_F(MigrationTest, LegacyRows_KeepRepetitionsAndWeight)
{
    createLegacySchema();
    insertLegacyData();

    migrate();

    EXPECT_EQ(scalar("SELECT COUNT(*) FROM sets").toInt(), 2);
    EXPECT_EQ(scalar("SELECT repetitions FROM sets WHERE id = 1").toInt(), 10);
    EXPECT_DOUBLE_EQ(scalar("SELECT weight FROM sets WHERE id = 2").toDouble(), 85.5);
    EXPECT_EQ(scalar("SELECT completed FROM sets WHERE id = 1").toInt(), 1);
    EXPECT_EQ(scalar("SELECT name FROM workouts WHERE id = 1").toString(), "Push Day");
}

TEST_F(MigrationTest, LegacyWorkout_ReadsBackAsWeightedReps)
{
    createLegacySchema();
    insertLegacyData();

    migrate();

    auto workout = m_repo->findOne(WorkoutQuery().whereId(1));

    ASSERT_TRUE(workout.has_value());
    ASSERT_EQ(workout->exercises().size(), 1u);

    const Exercise& exercise = workout->exercises()[0];
    EXPECT_EQ(exercise.kind(), ExerciseKind::Strength);
    EXPECT_EQ(exercise.restSecondsForSet(0), 120);

    ASSERT_EQ(exercise.sets().size(), 2u);
    EXPECT_TRUE(exercise.sets()[0].isWeighted());
    EXPECT_EQ(exercise.sets()[0].metric(), SetMetric::Reps);
    EXPECT_EQ(exercise.sets()[0].loadType(), LoadType::External);
    EXPECT_EQ(exercise.sets()[0].repetitions(), 10);
    EXPECT_DOUBLE_EQ(exercise.sets()[0].weight(), 80.0);
    EXPECT_DOUBLE_EQ(exercise.totalWeight(), 1484.0);
}

TEST_F(MigrationTest, LegacyDatabase_ReachesLatestSchemaVersion)
{
    createLegacySchema();
    insertLegacyData();

    migrate();

    EXPECT_EQ(m_schemaVersion, 3);
}

TEST_F(MigrationTest, Migration_IsIdempotent)
{
    createLegacySchema();
    insertLegacyData();
    migrate();

    MigrationRunner second(*m_dbStorage);
    m_repo->registerMigrations(second);

    EXPECT_TRUE(second.run());
    EXPECT_EQ(second.currentVersion(), 3);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM sets").toInt(), 2);
    EXPECT_EQ(scalar("SELECT metric FROM sets WHERE id = 1").toString(), "reps");
}

TEST_F(MigrationTest, FreshDatabase_HasAllColumnsAndLatestVersion)
{
    migrate();

    const QStringList setColumns = columnsOf(SetSerializer::table);
    EXPECT_TRUE(setColumns.contains(SetSerializer::metric_key));
    EXPECT_TRUE(setColumns.contains(SetSerializer::rest_seconds_override_key));
    EXPECT_TRUE(columnsOf(ExerciseSerializer::table).contains(ExerciseSerializer::kind_key));
    EXPECT_EQ(m_schemaVersion, 3);
}

TEST_F(MigrationTest, FreshDatabase_RoundtripsTimedAndDistanceSets)
{
    migrate();

    Workout w("Hybrid", QDateTime::currentDateTime());

    Exercise tabata("Burpees", 60);
    tabata.setKind(ExerciseKind::Interval);
    Set work = Set::createDuration(20);
    work.setRestSecondsOverride(10);
    tabata.addSet(work);
    w.addExercise(tabata);

    Exercise run("Run", 0);
    run.setKind(ExerciseKind::Cardio);
    run.addSet(Set::createDistance(5000.0, 1500));
    w.addExercise(run);

    int id = m_repo->save(w);
    ASSERT_GT(id, 0);

    auto loaded = m_repo->findOne(WorkoutQuery().whereId(id));
    ASSERT_TRUE(loaded.has_value());
    ASSERT_EQ(loaded->exercises().size(), 2u);

    const Exercise& loadedTabata = loaded->exercises()[0];
    EXPECT_EQ(loadedTabata.kind(), ExerciseKind::Interval);
    ASSERT_EQ(loadedTabata.sets().size(), 1u);
    EXPECT_EQ(loadedTabata.sets()[0].metric(), SetMetric::Duration);
    EXPECT_EQ(loadedTabata.sets()[0].durationSeconds(), 20);
    EXPECT_EQ(loadedTabata.restSecondsForSet(0), 10);

    const Exercise& loadedRun = loaded->exercises()[1];
    EXPECT_EQ(loadedRun.kind(), ExerciseKind::Cardio);
    ASSERT_EQ(loadedRun.sets().size(), 1u);
    EXPECT_EQ(loadedRun.sets()[0].metric(), SetMetric::Distance);
    EXPECT_DOUBLE_EQ(loadedRun.sets()[0].distanceMeters(), 5000.0);
    EXPECT_EQ(loadedRun.sets()[0].durationSeconds(), 1500);
    EXPECT_DOUBLE_EQ(loaded->totalWeight(), 0.0);
}
