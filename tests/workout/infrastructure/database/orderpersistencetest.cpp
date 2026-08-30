#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>
#include <gtest/gtest.h>

class OrderPersistenceTest : public ::testing::Test
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
        m_repo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
        ASSERT_TRUE(m_repo->createTables());

        MigrationRunner runner(*m_dbStorage);
        m_repo->registerMigrations(runner);
        ASSERT_TRUE(runner.run());
    }

    void TearDown() override
    {
        m_repo.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    Workout threeExerciseWorkout()
    {
        Workout workout("Pull", QDateTime::currentDateTime());
        workout.addExercise(Exercise::createAdHoc("Deadlift", ExerciseKind::Strength, 180));
        workout.addExercise(Exercise::createAdHoc("Barbell Row", ExerciseKind::Strength, 90));
        workout.addExercise(Exercise::createAdHoc("Curl", ExerciseKind::Strength, 60));
        return workout;
    }

    Workout reload(int id) const
    {
        auto loaded = m_repo->findOne(WorkoutQuery().whereId(id));
        return loaded.value_or(Workout());
    }

    QString m_connectionName = "workout_order_persistence_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_repo;
};

TEST_F(OrderPersistenceTest, ExerciseOrderSurvivesARoundTrip)
{
    const int id = m_repo->save(threeExerciseWorkout());

    const Workout loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 3u);
    EXPECT_EQ(loaded.exercises()[0].name(), "Deadlift");
    EXPECT_EQ(loaded.exercises()[1].name(), "Barbell Row");
    EXPECT_EQ(loaded.exercises()[2].name(), "Curl");
}

TEST_F(OrderPersistenceTest, ReorderingIsPersistedNotJustHeldInMemory)
{
    Workout workout = threeExerciseWorkout();
    const int id = m_repo->save(workout);

    Workout stored = reload(id);
    stored.moveExercise(2, 0);
    m_repo->save(stored);

    const Workout loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 3u);
    EXPECT_EQ(loaded.exercises()[0].name(), "Curl");
    EXPECT_EQ(loaded.exercises()[1].name(), "Deadlift");
    EXPECT_EQ(loaded.exercises()[2].name(), "Barbell Row");
    for (size_t i = 0; i < loaded.exercises().size(); ++i)
        EXPECT_EQ(loaded.exercises()[i].position(), static_cast<int>(i));
}

TEST_F(OrderPersistenceTest, AnExerciseInsertedInTheMiddleKeepsItsPlace)
{
    Workout workout = threeExerciseWorkout();
    const int id = m_repo->save(workout);

    Workout stored = reload(id);
    stored.addExercise(Exercise::createAdHoc("Face Pull", ExerciseKind::Strength, 45), 1);
    m_repo->save(stored);

    const Workout loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 4u);
    EXPECT_EQ(loaded.exercises()[0].name(), "Deadlift");
    EXPECT_EQ(loaded.exercises()[1].name(), "Face Pull");
    EXPECT_EQ(loaded.exercises()[2].name(), "Barbell Row");
    EXPECT_EQ(loaded.exercises()[3].name(), "Curl");
}

TEST_F(OrderPersistenceTest, SetOrderSurvivesAReorder)
{
    Workout workout("Push", QDateTime::currentDateTime());
    Exercise bench = Exercise::createAdHoc("Bench Press", ExerciseKind::Strength, 120);
    bench.addSet(Set(5, 60.0));
    bench.addSet(Set(5, 70.0));
    bench.addSet(Set(5, 80.0));
    workout.addExercise(bench);

    const int id = m_repo->save(workout);

    Workout stored = reload(id);
    stored.exercises()[0].moveSet(2, 0);
    m_repo->save(stored);

    const Workout loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 1u);
    ASSERT_EQ(loaded.exercises()[0].sets().size(), 3u);
    EXPECT_DOUBLE_EQ(loaded.exercises()[0].sets()[0].weight(), 80.0);
    EXPECT_DOUBLE_EQ(loaded.exercises()[0].sets()[1].weight(), 60.0);
    EXPECT_DOUBLE_EQ(loaded.exercises()[0].sets()[2].weight(), 70.0);
    for (size_t i = 0; i < loaded.exercises()[0].sets().size(); ++i)
        EXPECT_EQ(loaded.exercises()[0].sets()[i].position(), static_cast<int>(i));
}

TEST_F(OrderPersistenceTest, TheCatalogLinkIsPersisted)
{
    Workout workout("Push", QDateTime::currentDateTime());
    workout.addExercise(
        Exercise::createFromDefinition(42, "Barbell Back Squat", ExerciseKind::Strength, 180));

    const int id = m_repo->save(workout);
    const Workout loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 1u);
    ASSERT_TRUE(loaded.exercises()[0].hasDefinition());
    EXPECT_EQ(loaded.exercises()[0].definitionId().value(), 42);
}

TEST_F(OrderPersistenceTest, AnAdHocExerciseRoundTripsWithoutALink)
{
    Workout workout("Push", QDateTime::currentDateTime());
    workout.addExercise(Exercise::createAdHoc("Sandbag Carry", ExerciseKind::Cardio, 90));

    const int id = m_repo->save(workout);
    const Workout loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 1u);
    EXPECT_FALSE(loaded.exercises()[0].hasDefinition());
    EXPECT_EQ(loaded.exercises()[0].name(), "Sandbag Carry");
}

TEST_F(OrderPersistenceTest, DetachingTheLinkIsPersistedAsNull)
{
    Workout workout("Push", QDateTime::currentDateTime());
    workout.addExercise(
        Exercise::createFromDefinition(42, "Barbell Back Squat", ExerciseKind::Strength, 180));
    const int id = m_repo->save(workout);

    Workout stored = reload(id);
    stored.exercises()[0].clearDefinitionId();
    m_repo->save(stored);

    EXPECT_FALSE(reload(id).exercises()[0].hasDefinition());
}

TEST_F(OrderPersistenceTest, NotesArePersistedSeparatelyFromTheDescription)
{
    Workout workout("Push", QDateTime::currentDateTime());
    Exercise bench = Exercise::createAdHoc("Bench Press", ExerciseKind::Strength, 120);
    bench.setDescription("Flat barbell press off the chest");
    bench.setNotes("Left shoulder felt tight");
    workout.addExercise(bench);

    const int id = m_repo->save(workout);
    const Workout loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 1u);
    EXPECT_EQ(loaded.exercises()[0].description(), "Flat barbell press off the chest");
    EXPECT_EQ(loaded.exercises()[0].notes(), "Left shoulder felt tight");
}
