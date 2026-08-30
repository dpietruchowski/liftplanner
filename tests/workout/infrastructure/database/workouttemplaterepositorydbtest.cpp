#include "modules/workout/infrastructure/database/workouttemplaterepositorydb.h"
#include "modules/workout/domain/entities/setprescription.h"
#include "modules/workout/domain/entities/templateexercise.h"
#include "modules/workout/domain/entities/workouttemplate.h"
#include "modules/workout/domain/repositories/workouttemplatequery.h"
#include "modules/workout/infrastructure/serializers/workouttemplateserializer.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include <gtest/gtest.h>

class WorkoutTemplateRepositoryDbTest : public ::testing::Test
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
        m_repo = std::make_unique<WorkoutTemplateRepositoryDb>(*m_dbStorage);
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

    static TemplateExercise strengthExercise(int definitionId, int sets, int reps, double weight)
    {
        TemplateExercise exercise(definitionId);
        for (int i = 0; i < sets; ++i)
            exercise.addSet(SetPrescription(reps, weight));
        return exercise;
    }

    static WorkoutTemplate pushDay()
    {
        WorkoutTemplate workoutTemplate("Push Day");
        workoutTemplate.setNotes("Heavy on the first movement.");
        workoutTemplate.addExercise(strengthExercise(1, 3, 5, 100.0));
        workoutTemplate.addExercise(strengthExercise(2, 4, 10, 30.0));
        workoutTemplate.addExercise(strengthExercise(3, 3, 12, 20.0));
        return workoutTemplate;
    }

    WorkoutTemplate reload(int id) const
    {
        return m_repo->findOne(WorkoutTemplateQuery().whereId(id)).value_or(WorkoutTemplate());
    }

    int rowCount(const QString& table) const
    {
        QSqlQuery query(m_database);
        query.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(table));
        return query.next() ? query.value(0).toInt() : -1;
    }

    QString m_connectionName = "workout_template_repository_test";
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutTemplateRepositoryDb> m_repo;
};

TEST_F(WorkoutTemplateRepositoryDbTest, SavingAssignsAnId)
{
    EXPECT_GT(m_repo->save(pushDay()), 0);
}

TEST_F(WorkoutTemplateRepositoryDbTest, TheTemplateHeaderSurvivesARoundTrip)
{
    const int id = m_repo->save(pushDay());

    const WorkoutTemplate loaded = reload(id);

    EXPECT_EQ(loaded.id(), id);
    EXPECT_EQ(loaded.name(), "Push Day");
    EXPECT_EQ(loaded.notes(), "Heavy on the first movement.");
}

TEST_F(WorkoutTemplateRepositoryDbTest, ExerciseOrderSurvivesARoundTrip)
{
    const int id = m_repo->save(pushDay());

    const WorkoutTemplate loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 3u);
    EXPECT_EQ(loaded.exercises()[0].definitionId(), 1);
    EXPECT_EQ(loaded.exercises()[1].definitionId(), 2);
    EXPECT_EQ(loaded.exercises()[2].definitionId(), 3);
    EXPECT_EQ(loaded.exercises()[0].position(), 0);
    EXPECT_EQ(loaded.exercises()[2].position(), 2);
}

TEST_F(WorkoutTemplateRepositoryDbTest, AReorderedTemplateComesBackInTheNewOrder)
{
    WorkoutTemplate workoutTemplate = pushDay();
    workoutTemplate.moveExercise(2, 0);
    const int id = m_repo->save(workoutTemplate);

    const WorkoutTemplate loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 3u);
    EXPECT_EQ(loaded.exercises()[0].definitionId(), 3);
    EXPECT_EQ(loaded.exercises()[1].definitionId(), 1);
}

TEST_F(WorkoutTemplateRepositoryDbTest, PrescriptionsSurviveARoundTrip)
{
    const int id = m_repo->save(pushDay());

    const WorkoutTemplate loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 3u);
    ASSERT_EQ(loaded.exercises()[0].sets().size(), 3u);
    EXPECT_EQ(loaded.exercises()[0].sets()[0].repetitions(), 5);
    EXPECT_DOUBLE_EQ(loaded.exercises()[0].sets()[0].weight(), 100.0);
    EXPECT_EQ(loaded.exercises()[1].sets().size(), 4u);
    EXPECT_EQ(loaded.exercises()[1].sets()[0].repetitions(), 10);
}

TEST_F(WorkoutTemplateRepositoryDbTest, EveryPrescriptionFieldSurvivesARoundTrip)
{
    WorkoutTemplate workoutTemplate("Conditioning");
    TemplateExercise exercise(7);
    exercise.setNotes("Steady pace.");
    exercise.setRestSecondsOverride(45);

    SetPrescription distance = SetPrescription::createDistance(5000.0, 1500);
    distance.setRestSecondsOverride(120);
    exercise.addSet(distance);
    exercise.addSet(SetPrescription::createDuration(90));

    workoutTemplate.addExercise(exercise);
    const int id = m_repo->save(workoutTemplate);

    const WorkoutTemplate loaded = reload(id);

    ASSERT_EQ(loaded.exercises().size(), 1u);
    const TemplateExercise& reloaded = loaded.exercises().front();
    EXPECT_EQ(reloaded.notes(), "Steady pace.");
    EXPECT_EQ(reloaded.restSecondsOverride(), 45);

    ASSERT_EQ(reloaded.sets().size(), 2u);
    EXPECT_EQ(reloaded.sets()[0].metric(), SetMetric::Distance);
    EXPECT_DOUBLE_EQ(reloaded.sets()[0].distanceMeters(), 5000.0);
    EXPECT_EQ(reloaded.sets()[0].durationSeconds(), 1500);
    EXPECT_EQ(reloaded.sets()[0].restSecondsOverride(), 120);
    EXPECT_EQ(reloaded.sets()[1].metric(), SetMetric::Duration);
    EXPECT_EQ(reloaded.sets()[1].durationSeconds(), 90);
    EXPECT_EQ(reloaded.sets()[1].loadType(), LoadType::None);
}

TEST_F(WorkoutTemplateRepositoryDbTest, PrescriptionOrderSurvivesARoundTrip)
{
    WorkoutTemplate workoutTemplate("Ladder");
    TemplateExercise exercise(4);
    exercise.addSet(SetPrescription(12, 40.0));
    exercise.addSet(SetPrescription(10, 50.0));
    exercise.addSet(SetPrescription(8, 60.0));
    workoutTemplate.addExercise(exercise);

    const WorkoutTemplate loaded = reload(m_repo->save(workoutTemplate));

    ASSERT_EQ(loaded.exercises().size(), 1u);
    const auto& sets = loaded.exercises().front().sets();
    ASSERT_EQ(sets.size(), 3u);
    EXPECT_EQ(sets[0].repetitions(), 12);
    EXPECT_EQ(sets[1].repetitions(), 10);
    EXPECT_EQ(sets[2].repetitions(), 8);
}

TEST_F(WorkoutTemplateRepositoryDbTest, ResavingReplacesChildrenInsteadOfAppending)
{
    const int id = m_repo->save(pushDay());

    WorkoutTemplate trimmed = reload(id);
    trimmed.removeExercise(2);
    m_repo->save(trimmed);

    EXPECT_EQ(reload(id).exercises().size(), 2u);
    EXPECT_EQ(rowCount(TemplateExerciseSerializer::table), 2);
    EXPECT_EQ(rowCount(SetPrescriptionSerializer::table), 7);
}

TEST_F(WorkoutTemplateRepositoryDbTest, SavingAnExistingIdUpdatesInPlace)
{
    const int id = m_repo->save(pushDay());

    WorkoutTemplate renamed = reload(id);
    renamed.setName("Push Day A");
    const int second = m_repo->save(renamed);

    EXPECT_EQ(second, id);
    EXPECT_EQ(m_repo->count(WorkoutTemplateQuery()), 1);
    EXPECT_EQ(reload(id).name(), "Push Day A");
}

TEST_F(WorkoutTemplateRepositoryDbTest, FindAllLoadsChildrenForEveryTemplate)
{
    m_repo->save(pushDay());

    WorkoutTemplate pullDay("Pull Day");
    pullDay.addExercise(strengthExercise(9, 2, 8, 70.0));
    m_repo->save(pullDay);

    const auto all = m_repo->findAll(WorkoutTemplateQuery().orderByName(SortDirection::Ascending));

    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].name(), "Pull Day");
    EXPECT_EQ(all[0].exercises().size(), 1u);
    EXPECT_EQ(all[1].exercises().size(), 3u);
    EXPECT_EQ(all[1].exercises()[0].sets().size(), 3u);
}

TEST_F(WorkoutTemplateRepositoryDbTest, OrderByNameSortsBothWays)
{
    m_repo->save(pushDay());
    WorkoutTemplate pullDay("Pull Day");
    pullDay.addExercise(strengthExercise(9, 1, 8, 70.0));
    m_repo->save(pullDay);

    const auto descending
        = m_repo->findAll(WorkoutTemplateQuery().orderByName(SortDirection::Descending));

    ASSERT_EQ(descending.size(), 2u);
    EXPECT_EQ(descending[0].name(), "Push Day");
}

TEST_F(WorkoutTemplateRepositoryDbTest, NameContainsFilters)
{
    m_repo->save(pushDay());
    WorkoutTemplate pullDay("Pull Day");
    pullDay.addExercise(strengthExercise(9, 1, 8, 70.0));
    m_repo->save(pullDay);

    const auto found = m_repo->findAll(WorkoutTemplateQuery().whereNameContains("Pull"));

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name(), "Pull Day");
}

TEST_F(WorkoutTemplateRepositoryDbTest, ReferencesDefinitionFindsTemplatesUsingAnExercise)
{
    m_repo->save(pushDay());

    WorkoutTemplate pullDay("Pull Day");
    pullDay.addExercise(strengthExercise(9, 2, 8, 70.0));
    m_repo->save(pullDay);

    const auto found = m_repo->findAll(WorkoutTemplateQuery().whereReferencesDefinition(2));

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name(), "Push Day");
    EXPECT_EQ(m_repo->count(WorkoutTemplateQuery().whereReferencesDefinition(9)), 1);
    EXPECT_EQ(m_repo->count(WorkoutTemplateQuery().whereReferencesDefinition(404)), 0);
}

TEST_F(WorkoutTemplateRepositoryDbTest, ReferencesDefinitionMatchesATemplateOnlyOnce)
{
    WorkoutTemplate repeated("Superset");
    repeated.addExercise(strengthExercise(5, 3, 10, 40.0));
    repeated.addExercise(strengthExercise(5, 3, 10, 45.0));
    m_repo->save(repeated);

    EXPECT_EQ(m_repo->count(WorkoutTemplateQuery().whereReferencesDefinition(5)), 1);
}

TEST_F(WorkoutTemplateRepositoryDbTest, NameContainsKeepsWorkingAlongsideAnotherFilter)
{
    m_repo->save(pushDay());

    WorkoutTemplate other("Push Day Light");
    other.addExercise(strengthExercise(9, 1, 8, 20.0));
    m_repo->save(other);

    const auto found = m_repo->findAll(
        WorkoutTemplateQuery().whereNameContains("Push").whereReferencesDefinition(9));

    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found[0].name(), "Push Day Light");
}

TEST_F(WorkoutTemplateRepositoryDbTest, CountAndExistsAgreeWithTheFilter)
{
    const int id = m_repo->save(pushDay());

    EXPECT_EQ(m_repo->count(WorkoutTemplateQuery()), 1);
    EXPECT_TRUE(m_repo->exists(WorkoutTemplateQuery().whereId(id)));
    EXPECT_FALSE(m_repo->exists(WorkoutTemplateQuery().whereName("Leg Day")));
}

TEST_F(WorkoutTemplateRepositoryDbTest, RemovingATemplateCascadesToExercisesAndPrescriptions)
{
    const int id = m_repo->save(pushDay());

    EXPECT_TRUE(m_repo->remove(WorkoutTemplateQuery().whereId(id)));

    EXPECT_EQ(m_repo->count(WorkoutTemplateQuery()), 0);
    EXPECT_EQ(rowCount(TemplateExerciseSerializer::table), 0);
    EXPECT_EQ(rowCount(SetPrescriptionSerializer::table), 0);
}

TEST_F(WorkoutTemplateRepositoryDbTest, RemoveReturnsFalseWhenNothingMatches)
{
    m_repo->save(pushDay());

    EXPECT_FALSE(m_repo->remove(WorkoutTemplateQuery().whereName("Leg Day")));
}

TEST_F(WorkoutTemplateRepositoryDbTest, FindOneReturnsNulloptWhenNothingMatches)
{
    EXPECT_FALSE(m_repo->findOne(WorkoutTemplateQuery().whereId(404)).has_value());
}

TEST_F(WorkoutTemplateRepositoryDbTest, ATemplateTurnsBackIntoAnExercise)
{
    const int id = m_repo->save(pushDay());

    const WorkoutTemplate loaded = reload(id);
    const Exercise exercise
        = loaded.exercises()[0].toExercise("Bench Press", ExerciseKind::Strength, 180);

    EXPECT_EQ(exercise.name(), "Bench Press");
    EXPECT_EQ(exercise.definitionId(), 1);
    EXPECT_EQ(exercise.restSeconds(), 180);
    ASSERT_EQ(exercise.sets().size(), 3u);
    EXPECT_EQ(exercise.sets()[0].repetitions(), 5);
}
