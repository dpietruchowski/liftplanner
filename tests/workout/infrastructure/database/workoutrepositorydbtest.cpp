#include <gtest/gtest.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"

class WorkoutRepositoryDbTest : public ::testing::Test
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
        ASSERT_TRUE(m_repo->createTable());
    }

    void TearDown() override
    {
        m_repo.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    Workout makeWorkout(const QString &name)
    {
        return Workout(name, QDateTime::currentDateTime());
    }

    QString m_connectionName = "workout_repo_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_repo;
};

TEST_F(WorkoutRepositoryDbTest, Save_NewWorkout_ReturnsValidId)
{
    Workout w = makeWorkout("Push Day");

    int id = m_repo->save(w);

    EXPECT_GT(id, 0);
}

TEST_F(WorkoutRepositoryDbTest, Save_And_FindOne_ById)
{
    Workout w = makeWorkout("Leg Day");
    int id = m_repo->save(w);

    auto found = m_repo->findOne(WorkoutQuery().whereId(id));

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id(), id);
    EXPECT_EQ(found->name(), "Leg Day");
}

TEST_F(WorkoutRepositoryDbTest, FindAll_ReturnsAllWorkouts)
{
    m_repo->save(makeWorkout("Workout A"));
    m_repo->save(makeWorkout("Workout B"));
    m_repo->save(makeWorkout("Workout C"));

    auto all = m_repo->findAll(WorkoutQuery());

    EXPECT_EQ(all.size(), 3u);
}

TEST_F(WorkoutRepositoryDbTest, FindAll_WithNameFilter)
{
    m_repo->save(makeWorkout("Push"));
    m_repo->save(makeWorkout("Pull"));
    m_repo->save(makeWorkout("Push"));

    auto results = m_repo->findAll(WorkoutQuery().whereName("Push"));

    EXPECT_EQ(results.size(), 2u);
    for (const auto &w : results)
        EXPECT_EQ(w.name(), "Push");
}

TEST_F(WorkoutRepositoryDbTest, FindAll_WithLimit)
{
    for (int i = 0; i < 5; ++i)
        m_repo->save(makeWorkout("W" + QString::number(i)));

    auto results = m_repo->findAll(WorkoutQuery().withLimit(3));

    EXPECT_EQ(results.size(), 3u);
}

TEST_F(WorkoutRepositoryDbTest, Save_Update_ExistingWorkout)
{
    Workout w = makeWorkout("Original");
    int id = m_repo->save(w);

    w.setId(id);
    w.setName("Updated");
    m_repo->save(w);

    auto found = m_repo->findOne(WorkoutQuery().whereId(id));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name(), "Updated");

    EXPECT_EQ(m_repo->count(WorkoutQuery()), 1);
}

TEST_F(WorkoutRepositoryDbTest, Remove_ById)
{
    int id = m_repo->save(makeWorkout("ToDelete"));
    EXPECT_EQ(m_repo->count(WorkoutQuery()), 1);

    bool removed = m_repo->remove(WorkoutQuery().whereId(id));

    EXPECT_TRUE(removed);
    EXPECT_EQ(m_repo->count(WorkoutQuery()), 0);
}

TEST_F(WorkoutRepositoryDbTest, Count_ReturnsCorrectCount)
{
    EXPECT_EQ(m_repo->count(WorkoutQuery()), 0);

    m_repo->save(makeWorkout("A"));
    m_repo->save(makeWorkout("B"));

    EXPECT_EQ(m_repo->count(WorkoutQuery()), 2);
}

TEST_F(WorkoutRepositoryDbTest, Exists_ReturnsTrueWhenFound)
{
    int id = m_repo->save(makeWorkout("Exists"));

    EXPECT_TRUE(m_repo->exists(WorkoutQuery().whereId(id)));
    EXPECT_FALSE(m_repo->exists(WorkoutQuery().whereId(9999)));
}

TEST_F(WorkoutRepositoryDbTest, FindOne_ReturnsNullopt_WhenNotFound)
{
    auto found = m_repo->findOne(WorkoutQuery().whereId(9999));

    EXPECT_FALSE(found.has_value());
}

TEST_F(WorkoutRepositoryDbTest, StartedTimeFilter_Null)
{
    Workout routine = makeWorkout("Routine");
    m_repo->save(routine);

    Workout started = makeWorkout("Started");
    started.setStartedTime(QDateTime::currentDateTime());
    m_repo->save(started);

    auto routines = m_repo->findAll(WorkoutQuery().whereStartedTimeIsNull());
    EXPECT_EQ(routines.size(), 1u);
    EXPECT_EQ(routines[0].name(), "Routine");

    auto history = m_repo->findAll(WorkoutQuery().whereStartedTimeIsNotNull());
    EXPECT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0].name(), "Started");
}
