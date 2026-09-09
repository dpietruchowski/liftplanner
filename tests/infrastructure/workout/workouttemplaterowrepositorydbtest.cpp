#include "infrastructure/workout/workouttemplaterowrepositorydb.h"
#include "domain/workout/setprescription.h"
#include "domain/workout/templateexercise.h"
#include "domain/workout/workouttemplate.h"
#include "domain/workout/workouttemplatequery.h"
#include "infrastructure/exercisecatalog/exercisedefinitionrepositorydb.h"
#include "infrastructure/workout/workouttemplaterepositorydb.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include <gtest/gtest.h>

class WorkoutTemplateRowRepositoryDbTest : public ::testing::Test
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
        m_definitions = std::make_unique<ExerciseDefinitionRepositoryDb>(*m_dbStorage);
        ASSERT_TRUE(m_definitions->createTables());
        m_templates = std::make_unique<WorkoutTemplateRepositoryDb>(*m_dbStorage);
        ASSERT_TRUE(m_templates->createTables());
        m_rows = std::make_unique<WorkoutTemplateRowRepositoryDb>(*m_dbStorage);
    }

    void TearDown() override
    {
        m_rows.reset();
        m_templates.reset();
        m_definitions.reset();
        m_dbStorage.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    int seedDefinition(const QString& slug, const QString& name)
    {
        ExerciseDefinition definition(slug, name, ExerciseKind::Strength);
        return m_definitions->save(definition);
    }

    static TemplateExercise exerciseOf(int definitionId, int prescriptions)
    {
        TemplateExercise exercise(definitionId);
        for (int i = 0; i < prescriptions; ++i)
            exercise.addSet(SetPrescription(5, 100.0));
        return exercise;
    }

    std::vector<WorkoutTemplateRow> rows(const WorkoutTemplateQuery& query) const
    {
        return m_rows->findAll(query);
    }

    static WorkoutTemplateQuery byName(SortDirection direction = SortDirection::Ascending)
    {
        WorkoutTemplateQuery query;
        query.orderByName(direction);
        return query;
    }

    QString m_connectionName = "workout_template_row_repository_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<ExerciseDefinitionRepositoryDb> m_definitions;
    std::unique_ptr<WorkoutTemplateRepositoryDb> m_templates;
    std::unique_ptr<WorkoutTemplateRowRepositoryDb> m_rows;
};

TEST_F(WorkoutTemplateRowRepositoryDbTest, ARowCarriesTheCountsAndTheExerciseNames)
{
    const int squat = seedDefinition("back-squat", "Back Squat");
    const int bench = seedDefinition("bench-press", "Bench Press");

    WorkoutTemplate pushDay("Push Day");
    pushDay.setNotes("Heavy on the first movement.");
    pushDay.addExercise(exerciseOf(squat, 3));
    pushDay.addExercise(exerciseOf(bench, 4));
    m_templates->save(pushDay);

    const auto found = rows(byName());

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name, "Push Day");
    EXPECT_EQ(found[0].notes, "Heavy on the first movement.");
    EXPECT_EQ(found[0].exerciseCount, 2);
    EXPECT_EQ(found[0].setCount, 7);
    EXPECT_EQ(found[0].exerciseNames, QStringList({ "Back Squat", "Bench Press" }));
    EXPECT_TRUE(found[0].complete);
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, ATemplateWithoutExercisesStillGetsARow)
{
    m_templates->save(WorkoutTemplate("Empty Day"));

    const auto found = rows(byName());

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name, "Empty Day");
    EXPECT_EQ(found[0].exerciseCount, 0);
    EXPECT_EQ(found[0].setCount, 0);
    EXPECT_TRUE(found[0].exerciseNames.isEmpty());
    EXPECT_TRUE(found[0].complete);
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, AnExerciseWithoutADefinitionMarksTheRowIncomplete)
{
    const int squat = seedDefinition("back-squat", "Back Squat");

    WorkoutTemplate mixed("Mixed Day");
    mixed.addExercise(exerciseOf(squat, 2));
    mixed.addExercise(exerciseOf(404, 3));
    m_templates->save(mixed);

    const auto found = rows(byName());

    ASSERT_EQ(found.size(), 1u);
    EXPECT_FALSE(found[0].complete);
    EXPECT_EQ(found[0].exerciseCount, 2);
    EXPECT_EQ(found[0].setCount, 5);
    EXPECT_EQ(found[0].exerciseNames, QStringList({ "Back Squat" }));
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, AMissingNotesColumnComesBackEmptyNotNull)
{
    m_templates->save(WorkoutTemplate("No Notes"));

    const auto found = rows(byName());

    ASSERT_EQ(found.size(), 1u);
    EXPECT_TRUE(found[0].notes.isEmpty());
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, OrderByNameSortsBothWays)
{
    m_templates->save(WorkoutTemplate("Push Day"));
    m_templates->save(WorkoutTemplate("Pull Day"));

    const auto ascending = rows(byName());
    const auto descending = rows(byName(SortDirection::Descending));

    ASSERT_EQ(ascending.size(), 2u);
    EXPECT_EQ(ascending[0].name, "Pull Day");
    EXPECT_EQ(descending[0].name, "Push Day");
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, ALimitCutsTemplatesNotJoinedRows)
{
    const int squat = seedDefinition("back-squat", "Back Squat");

    WorkoutTemplate first("A Day");
    first.addExercise(exerciseOf(squat, 3));
    first.addExercise(exerciseOf(squat, 3));
    m_templates->save(first);

    WorkoutTemplate second("B Day");
    second.addExercise(exerciseOf(squat, 3));
    m_templates->save(second);

    WorkoutTemplateQuery query = byName();
    query.withLimit(1);

    const auto found = rows(query);

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name, "A Day");
    EXPECT_EQ(found[0].exerciseCount, 2);
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, TheLimitFollowsTheOrderingNotTheInsertionOrder)
{
    m_templates->save(WorkoutTemplate("B Day"));
    m_templates->save(WorkoutTemplate("A Day"));

    WorkoutTemplateQuery query = byName(SortDirection::Descending);
    query.withLimit(1);

    const auto found = rows(query);

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name, "B Day");
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, AnOffsetSkipsWholeTemplates)
{
    m_templates->save(WorkoutTemplate("A Day"));
    m_templates->save(WorkoutTemplate("B Day"));
    m_templates->save(WorkoutTemplate("C Day"));

    WorkoutTemplateQuery query = byName();
    query.withLimit(1).withOffset(1);

    const auto found = rows(query);

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name, "B Day");
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, NameContainsFilters)
{
    m_templates->save(WorkoutTemplate("Push Day"));
    m_templates->save(WorkoutTemplate("Pull Day"));

    WorkoutTemplateQuery query = byName();
    query.whereNameContains("Pull");

    const auto found = rows(query);

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name, "Pull Day");
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, ReferencesDefinitionFindsTemplatesUsingAnExercise)
{
    const int squat = seedDefinition("back-squat", "Back Squat");
    const int row = seedDefinition("barbell-row", "Barbell Row");

    WorkoutTemplate pushDay("Push Day");
    pushDay.addExercise(exerciseOf(squat, 1));
    m_templates->save(pushDay);

    WorkoutTemplate pullDay("Pull Day");
    pullDay.addExercise(exerciseOf(row, 1));
    m_templates->save(pullDay);

    WorkoutTemplateQuery query = byName();
    query.whereReferencesDefinition(squat);

    const auto found = rows(query);

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name, "Push Day");
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, ReferencesDefinitionMatchesATemplateOnlyOnce)
{
    const int squat = seedDefinition("back-squat", "Back Squat");

    WorkoutTemplate superset("Superset");
    superset.addExercise(exerciseOf(squat, 3));
    superset.addExercise(exerciseOf(squat, 3));
    m_templates->save(superset);

    WorkoutTemplateQuery query = byName();
    query.whereReferencesDefinition(squat);

    const auto found = rows(query);

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].exerciseCount, 2);
    EXPECT_EQ(found[0].setCount, 6);
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, NameContainsKeepsWorkingAlongsideAnotherFilter)
{
    const int squat = seedDefinition("back-squat", "Back Squat");
    const int row = seedDefinition("barbell-row", "Barbell Row");

    WorkoutTemplate pushDay("Push Day");
    pushDay.addExercise(exerciseOf(squat, 1));
    m_templates->save(pushDay);

    WorkoutTemplate light("Push Day Light");
    light.addExercise(exerciseOf(row, 1));
    m_templates->save(light);

    WorkoutTemplateQuery query = byName();
    query.whereNameContains("Push").whereReferencesDefinition(row);

    const auto found = rows(query);

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name, "Push Day Light");
}

TEST_F(WorkoutTemplateRowRepositoryDbTest, AnEmptyDatabaseYieldsNoRows)
{
    EXPECT_TRUE(rows(byName()).empty());
}
