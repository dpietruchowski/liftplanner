#include <gtest/gtest.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
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

    Workout makeWorkout(const QString &name)
    {
        return Workout(name, QDateTime::currentDateTime());
    }

    Workout makeFullWorkout(const QString &name)
    {
        Workout w(name, QDateTime::currentDateTime());

        Exercise e1("Bench Press", 120);
        e1.addSet(Set(10, 80));
        e1.addSet(Set(8, 85));
        w.addExercise(e1);

        Exercise e2("Squat", 180);
        e2.addSet(Set(5, 100));
        w.addExercise(e2);

        return w;
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

TEST_F(WorkoutRepositoryDbTest, Save_FullAggregate_LoadsExercisesAndSets)
{
    Workout w = makeFullWorkout("Full Day");
    int id = m_repo->save(w);

    auto found = m_repo->findOne(WorkoutQuery().whereId(id));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name(), "Full Day");
    ASSERT_EQ(found->exercises().size(), 2u);

    const auto &e1 = found->exercises()[0];
    EXPECT_EQ(e1.name(), "Bench Press");
    EXPECT_EQ(e1.restSeconds(), 120);
    ASSERT_EQ(e1.sets().size(), 2u);
    EXPECT_EQ(e1.sets()[0].repetitions(), 10);
    EXPECT_EQ(e1.sets()[0].weight(), 80);
    EXPECT_EQ(e1.sets()[1].repetitions(), 8);
    EXPECT_EQ(e1.sets()[1].weight(), 85);

    const auto &e2 = found->exercises()[1];
    EXPECT_EQ(e2.name(), "Squat");
    ASSERT_EQ(e2.sets().size(), 1u);
    EXPECT_EQ(e2.sets()[0].repetitions(), 5);
    EXPECT_EQ(e2.sets()[0].weight(), 100);
}

TEST_F(WorkoutRepositoryDbTest, FindAll_LoadsChildrenForAll)
{
    m_repo->save(makeFullWorkout("W1"));
    m_repo->save(makeFullWorkout("W2"));

    auto all = m_repo->findAll(WorkoutQuery());
    EXPECT_EQ(all.size(), 2u);

    for (const auto &w : all)
    {
        EXPECT_EQ(w.exercises().size(), 2u);
        EXPECT_FALSE(w.exercises()[0].sets().empty());
    }
}

TEST_F(WorkoutRepositoryDbTest, Remove_CascadesDeleteToChildren)
{
    int id = m_repo->save(makeFullWorkout("ToDelete"));
    EXPECT_EQ(m_repo->count(WorkoutQuery()), 1);

    m_repo->remove(WorkoutQuery().whereId(id));

    EXPECT_EQ(m_repo->count(WorkoutQuery()), 0);

    // Re-save to verify tables are intact
    int id2 = m_repo->save(makeFullWorkout("After"));
    auto found = m_repo->findOne(WorkoutQuery().whereId(id2));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->exercises().size(), 2u);
}

TEST_F(WorkoutRepositoryDbTest, PlannedTimeFilter_FindsPlannedWorkouts)
{
    Workout planned = makeWorkout("Planned");
    planned.setPlannedTime(QDateTime::currentDateTime().addDays(1));
    m_repo->save(planned);

    Workout noPlanned = makeWorkout("NoPlanned");
    m_repo->save(noPlanned);

    auto withPlanned = m_repo->findAll(WorkoutQuery().wherePlannedTimeIsNotNull());
    EXPECT_EQ(withPlanned.size(), 1u);
    EXPECT_EQ(withPlanned[0].name(), "Planned");

    auto withoutPlanned = m_repo->findAll(WorkoutQuery().wherePlannedTimeIsNull());
    EXPECT_EQ(withoutPlanned.size(), 1u);
    EXPECT_EQ(withoutPlanned[0].name(), "NoPlanned");
}

TEST_F(WorkoutRepositoryDbTest, OrderByPlannedTime_Ascending)
{
    Workout w1 = makeWorkout("Later");
    w1.setPlannedTime(QDateTime::currentDateTime().addDays(3));
    m_repo->save(w1);

    Workout w2 = makeWorkout("Sooner");
    w2.setPlannedTime(QDateTime::currentDateTime().addDays(1));
    m_repo->save(w2);

    Workout w3 = makeWorkout("Middle");
    w3.setPlannedTime(QDateTime::currentDateTime().addDays(2));
    m_repo->save(w3);

    auto results = m_repo->findAll(WorkoutQuery().orderByPlannedTime(SortDirection::Ascending));
    ASSERT_EQ(results.size(), 3u);
    EXPECT_EQ(results[0].name(), "Sooner");
    EXPECT_EQ(results[1].name(), "Middle");
    EXPECT_EQ(results[2].name(), "Later");
}

TEST_F(WorkoutRepositoryDbTest, OrderByPlannedTime_Descending)
{
    Workout w1 = makeWorkout("Later");
    w1.setPlannedTime(QDateTime::currentDateTime().addDays(3));
    m_repo->save(w1);

    Workout w2 = makeWorkout("Sooner");
    w2.setPlannedTime(QDateTime::currentDateTime().addDays(1));
    m_repo->save(w2);

    auto results = m_repo->findAll(WorkoutQuery().orderByPlannedTime(SortDirection::Descending));
    ASSERT_EQ(results.size(), 2u);
    EXPECT_EQ(results[0].name(), "Later");
    EXPECT_EQ(results[1].name(), "Sooner");
}

TEST_F(WorkoutRepositoryDbTest, PlannedVsStarted_SeparatesCorrectly)
{
    Workout planned = makeWorkout("Planned Day");
    planned.setPlannedTime(QDateTime::currentDateTime().addDays(1));
    m_repo->save(planned);

    Workout started = makeWorkout("Started Day");
    started.setStartedTime(QDateTime::currentDateTime());
    m_repo->save(started);

    Workout both = makeWorkout("Was Planned Now Started");
    both.setPlannedTime(QDateTime::currentDateTime().addDays(-1));
    both.setStartedTime(QDateTime::currentDateTime());
    m_repo->save(both);

    // Planned = startedTime IS NULL (not yet started)
    auto plannedResults = m_repo->findAll(WorkoutQuery().whereStartedTimeIsNull());
    EXPECT_EQ(plannedResults.size(), 1u);
    EXPECT_EQ(plannedResults[0].name(), "Planned Day");

    // History = startedTime IS NOT NULL
    auto historyResults = m_repo->findAll(WorkoutQuery().whereStartedTimeIsNotNull());
    EXPECT_EQ(historyResults.size(), 2u);
}

TEST_F(WorkoutRepositoryDbTest, Save_PreservesPlannedTime)
{
    QDateTime plannedTime = QDateTime::currentDateTime().addDays(5);
    Workout w = makeWorkout("Future Workout");
    w.setPlannedTime(plannedTime);
    int id = m_repo->save(w);

    auto found = m_repo->findOne(WorkoutQuery().whereId(id));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->plannedTime().toString(Qt::ISODate), plannedTime.toString(Qt::ISODate));
}
