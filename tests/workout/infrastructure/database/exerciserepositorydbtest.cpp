#include <gtest/gtest.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/repositories/exercisequery.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "modules/workout/infrastructure/database/exerciserepositorydb.h"

class ExerciseRepositoryDbTest : public ::testing::Test
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
        m_repo = std::make_unique<ExerciseRepositoryDb>(*m_dbStorage);

        ASSERT_TRUE(m_workoutRepo->createTable());
        ASSERT_TRUE(m_repo->createTable());

        Workout w("Test Workout", QDateTime::currentDateTime());
        m_workoutId = m_workoutRepo->save(w);
    }

    void TearDown() override
    {
        m_repo.reset();
        m_workoutRepo.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    Exercise makeExercise(const QString &name, int rest = 120)
    {
        Exercise e(name, rest);
        e.setWorkoutId(m_workoutId);
        return e;
    }

    QString m_connectionName = "exercise_repo_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<ExerciseRepositoryDb> m_repo;
    int m_workoutId = -1;
};

TEST_F(ExerciseRepositoryDbTest, Save_NewExercise_ReturnsValidId)
{
    Exercise e = makeExercise("Bench Press");

    int id = m_repo->save(e);

    EXPECT_GT(id, 0);
}

TEST_F(ExerciseRepositoryDbTest, Save_And_FindOne_ById)
{
    Exercise e = makeExercise("Squat", 180);
    e.setDescription("Barbell back squat");
    e.setYoutubeLink("https://youtube.com/squat");
    int id = m_repo->save(e);

    auto found = m_repo->findOne(ExerciseQuery().whereId(id));

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id(), id);
    EXPECT_EQ(found->name(), "Squat");
    EXPECT_EQ(found->description(), "Barbell back squat");
    EXPECT_EQ(found->restSeconds(), 180);
    EXPECT_EQ(found->youtubeLink(), "https://youtube.com/squat");
    EXPECT_EQ(found->workoutId(), m_workoutId);
}

TEST_F(ExerciseRepositoryDbTest, FindAll_ByWorkoutId)
{
    m_repo->save(makeExercise("Bench"));
    m_repo->save(makeExercise("Squat"));
    m_repo->save(makeExercise("Deadlift"));

    auto all = m_repo->findAll(ExerciseQuery().whereWorkoutId(m_workoutId));

    EXPECT_EQ(all.size(), 3u);
}

TEST_F(ExerciseRepositoryDbTest, FindAll_ByName)
{
    m_repo->save(makeExercise("Bench"));
    m_repo->save(makeExercise("Squat"));

    auto results = m_repo->findAll(ExerciseQuery().whereName("Bench"));

    EXPECT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].name(), "Bench");
}

TEST_F(ExerciseRepositoryDbTest, Save_Update_Existing)
{
    Exercise e = makeExercise("Original");
    int id = m_repo->save(e);

    e.setId(id);
    e.setName("Updated");
    e.setRestSeconds(90);
    m_repo->save(e);

    auto found = m_repo->findOne(ExerciseQuery().whereId(id));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name(), "Updated");
    EXPECT_EQ(found->restSeconds(), 90);

    EXPECT_EQ(m_repo->count(ExerciseQuery()), 1);
}

TEST_F(ExerciseRepositoryDbTest, Remove_ById)
{
    int id = m_repo->save(makeExercise("ToDelete"));
    EXPECT_EQ(m_repo->count(ExerciseQuery()), 1);

    bool removed = m_repo->remove(ExerciseQuery().whereId(id));

    EXPECT_TRUE(removed);
    EXPECT_EQ(m_repo->count(ExerciseQuery()), 0);
}

TEST_F(ExerciseRepositoryDbTest, Count_ReturnsCorrectCount)
{
    EXPECT_EQ(m_repo->count(ExerciseQuery()), 0);

    m_repo->save(makeExercise("A"));
    m_repo->save(makeExercise("B"));

    EXPECT_EQ(m_repo->count(ExerciseQuery()), 2);
}

TEST_F(ExerciseRepositoryDbTest, Exists_ReturnsTrueWhenFound)
{
    int id = m_repo->save(makeExercise("Exists"));

    EXPECT_TRUE(m_repo->exists(ExerciseQuery().whereId(id)));
    EXPECT_FALSE(m_repo->exists(ExerciseQuery().whereId(9999)));
}

TEST_F(ExerciseRepositoryDbTest, FindOne_ReturnsNullopt_WhenNotFound)
{
    auto found = m_repo->findOne(ExerciseQuery().whereId(9999));

    EXPECT_FALSE(found.has_value());
}

TEST_F(ExerciseRepositoryDbTest, CascadeDelete_WhenWorkoutRemoved)
{
    m_repo->save(makeExercise("Bench"));
    m_repo->save(makeExercise("Squat"));
    EXPECT_EQ(m_repo->count(ExerciseQuery()), 2);

    m_workoutRepo->remove(WorkoutQuery().whereId(m_workoutId));

    EXPECT_EQ(m_repo->count(ExerciseQuery()), 0);
}
