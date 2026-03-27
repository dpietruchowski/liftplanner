#include <gtest/gtest.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
#include "modules/workout/domain/repositories/setquery.h"
#include "modules/workout/domain/repositories/exercisequery.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "modules/workout/infrastructure/database/exerciserepositorydb.h"
#include "modules/workout/infrastructure/database/setrepositorydb.h"

class SetRepositoryDbTest : public ::testing::Test
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
        m_exerciseRepo = std::make_unique<ExerciseRepositoryDb>(*m_dbStorage);
        m_repo = std::make_unique<SetRepositoryDb>(*m_dbStorage);

        ASSERT_TRUE(m_workoutRepo->createTable());
        ASSERT_TRUE(m_exerciseRepo->createTable());
        ASSERT_TRUE(m_repo->createTable());

        Workout w("Test Workout", QDateTime::currentDateTime());
        m_workoutId = m_workoutRepo->save(w);

        Exercise e("Test Exercise", 120);
        e.setWorkoutId(m_workoutId);
        m_exerciseId = m_exerciseRepo->save(e);
    }

    void TearDown() override
    {
        m_repo.reset();
        m_exerciseRepo.reset();
        m_workoutRepo.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    Set makeSet(int reps, int weight)
    {
        Set s(reps, weight);
        s.setExerciseId(m_exerciseId);
        return s;
    }

    QString m_connectionName = "set_repo_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<ExerciseRepositoryDb> m_exerciseRepo;
    std::unique_ptr<SetRepositoryDb> m_repo;
    int m_workoutId = -1;
    int m_exerciseId = -1;
};

TEST_F(SetRepositoryDbTest, Save_NewSet_ReturnsValidId)
{
    Set s = makeSet(10, 80);

    int id = m_repo->save(s);

    EXPECT_GT(id, 0);
}

TEST_F(SetRepositoryDbTest, Save_And_FindOne_ById)
{
    Set s = makeSet(12, 100);
    int id = m_repo->save(s);

    auto found = m_repo->findOne(SetQuery().whereId(id));

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id(), id);
    EXPECT_EQ(found->exerciseId(), m_exerciseId);
    EXPECT_EQ(found->repetitions(), 12);
    EXPECT_EQ(found->weight(), 100);
}

TEST_F(SetRepositoryDbTest, FindAll_ByExerciseId)
{
    m_repo->save(makeSet(10, 80));
    m_repo->save(makeSet(8, 90));
    m_repo->save(makeSet(6, 100));

    auto all = m_repo->findAll(SetQuery().whereExerciseId(m_exerciseId));

    EXPECT_EQ(all.size(), 3u);
}

TEST_F(SetRepositoryDbTest, Save_Update_Existing)
{
    Set s = makeSet(10, 80);
    int id = m_repo->save(s);

    s.setId(id);
    s.setRepetitions(12);
    s.setWeight(85);
    m_repo->save(s);

    auto found = m_repo->findOne(SetQuery().whereId(id));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->repetitions(), 12);
    EXPECT_EQ(found->weight(), 85);

    EXPECT_EQ(m_repo->count(SetQuery()), 1);
}

TEST_F(SetRepositoryDbTest, Remove_ById)
{
    int id = m_repo->save(makeSet(5, 50));
    EXPECT_EQ(m_repo->count(SetQuery()), 1);

    bool removed = m_repo->remove(SetQuery().whereId(id));

    EXPECT_TRUE(removed);
    EXPECT_EQ(m_repo->count(SetQuery()), 0);
}

TEST_F(SetRepositoryDbTest, Count_ReturnsCorrectCount)
{
    EXPECT_EQ(m_repo->count(SetQuery()), 0);

    m_repo->save(makeSet(10, 80));
    m_repo->save(makeSet(8, 90));

    EXPECT_EQ(m_repo->count(SetQuery()), 2);
}

TEST_F(SetRepositoryDbTest, Exists_ReturnsTrueWhenFound)
{
    int id = m_repo->save(makeSet(10, 80));

    EXPECT_TRUE(m_repo->exists(SetQuery().whereId(id)));
    EXPECT_FALSE(m_repo->exists(SetQuery().whereId(9999)));
}

TEST_F(SetRepositoryDbTest, FindOne_ReturnsNullopt_WhenNotFound)
{
    auto found = m_repo->findOne(SetQuery().whereId(9999));

    EXPECT_FALSE(found.has_value());
}

TEST_F(SetRepositoryDbTest, CascadeDelete_WhenExerciseRemoved)
{
    m_repo->save(makeSet(10, 80));
    m_repo->save(makeSet(8, 90));
    EXPECT_EQ(m_repo->count(SetQuery()), 2);

    m_exerciseRepo->remove(ExerciseQuery().whereId(m_exerciseId));

    EXPECT_EQ(m_repo->count(SetQuery()), 0);
}

TEST_F(SetRepositoryDbTest, MultipleSets_DifferentExercises)
{
    Exercise e2("Exercise 2", 90);
    e2.setWorkoutId(m_workoutId);
    int exerciseId2 = m_exerciseRepo->save(e2);

    m_repo->save(makeSet(10, 80));
    m_repo->save(makeSet(8, 90));

    Set s3(12, 60);
    s3.setExerciseId(exerciseId2);
    m_repo->save(s3);

    auto setsForEx1 = m_repo->findAll(SetQuery().whereExerciseId(m_exerciseId));
    EXPECT_EQ(setsForEx1.size(), 2u);

    auto setsForEx2 = m_repo->findAll(SetQuery().whereExerciseId(exerciseId2));
    EXPECT_EQ(setsForEx2.size(), 1u);
}
