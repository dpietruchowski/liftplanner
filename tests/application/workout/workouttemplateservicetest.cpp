#include "application/workout/workouttemplateservice.h"
#include "domain/workout/exercisedefinitionlookup.h"
#include "domain/workout/set.h"
#include "domain/workout/setprescription.h"
#include "domain/workout/templateexercise.h"
#include "domain/workout/workout.h"
#include "domain/workout/workouttemplate.h"
#include "domain/workout/workouttemplatequery.h"
#include "domain/workout/workouttemplaterepository.h"
#include "domain/workout/workouttemplaterow.h"
#include "domain/workout/workouttemplaterowrepository.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class MockWorkoutTemplateRepository : public WorkoutTemplateRepository
{
public:
    MOCK_METHOD(std::optional<WorkoutTemplate>, findOne, (const WorkoutTemplateQuery& query),
                (const, override));
    MOCK_METHOD(int, save, (const WorkoutTemplate& workoutTemplate), (override));
    MOCK_METHOD(bool, remove, (const WorkoutTemplateQuery& query), (override));
    MOCK_METHOD(int, count, (const WorkoutTemplateQuery& query), (const, override));
    MOCK_METHOD(bool, exists, (const WorkoutTemplateQuery& query), (const, override));
};

class MockWorkoutTemplateRowRepository : public WorkoutTemplateRowRepository
{
public:
    MOCK_METHOD(std::vector<WorkoutTemplateRow>, findAll, (const WorkoutTemplateQuery& query),
                (const, override));
};

class FakeDefinitionLookup : public ExerciseDefinitionLookup
{
public:
    void add(int id, const QString& name, ExerciseKind kind, int restSeconds)
    {
        m_definitions[id] = ExerciseDefinitionSnapshot { id, name, kind, restSeconds };
    }

    std::optional<ExerciseDefinitionSnapshot> findDefinition(int definitionId) const override
    {
        const auto it = m_definitions.find(definitionId);
        if (it == m_definitions.end())
            return std::nullopt;
        return it->second;
    }

private:
    std::map<int, ExerciseDefinitionSnapshot> m_definitions;
};

class WorkoutTemplateServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_lookup.add(7, QStringLiteral("Back Squat"), ExerciseKind::Strength, 180);
        m_lookup.add(9, QStringLiteral("Plank"), ExerciseKind::Interval, 60);
        m_service = std::make_unique<WorkoutTemplateService>(m_repo, m_rowRepo, m_lookup, nullptr);
    }

    static TemplateExercise makeTemplateExercise(int definitionId, int setCount)
    {
        TemplateExercise exercise(definitionId);
        for (int i = 0; i < setCount; ++i)
            exercise.addSet(SetPrescription(5, 100.0));
        return exercise;
    }

    static WorkoutTemplate makeTemplate(int id, const QString& name)
    {
        WorkoutTemplate workoutTemplate(name);
        workoutTemplate.setId(id);
        workoutTemplate.addExercise(makeTemplateExercise(7, 3));
        return workoutTemplate;
    }

    Result<std::vector<WorkoutTemplateRow>> searchRows(const QString& text)
    {
        return m_service->searchRowsCore(text);
    }
    Result<int> save(const WorkoutTemplate& workoutTemplate)
    {
        return m_service->saveCore(workoutTemplate);
    }
    Result<int> saveFromWorkout(const Workout& workout, const QString& name)
    {
        return m_service->saveFromWorkoutCore(workout, name);
    }
    Result<int> duplicate(int id, const QString& name)
    {
        return m_service->duplicateCore(id, name);
    }
    Result<bool> remove(int id) { return m_service->removeCore(id); }
    Result<Workout> instantiate(int id, const QDateTime& plannedTime)
    {
        return m_service->instantiateCore(id, plannedTime);
    }

    MockWorkoutTemplateRepository m_repo;
    MockWorkoutTemplateRowRepository m_rowRepo;
    FakeDefinitionLookup m_lookup;
    std::unique_ptr<WorkoutTemplateService> m_service;
};

// --- searchRows ---

TEST_F(WorkoutTemplateServiceTest, SearchRowsOrdersByNameAscending)
{
    WorkoutTemplateQuery seen;
    EXPECT_CALL(m_rowRepo, findAll(_))
        .WillOnce(
            [&seen](const WorkoutTemplateQuery& query)
            {
                seen = query;
                return std::vector<WorkoutTemplateRow> {};
            });

    searchRows(QString());

    ASSERT_TRUE(seen.orderByNameDirection().has_value());
    EXPECT_EQ(seen.orderByNameDirection().value(), SortDirection::Ascending);
}

TEST_F(WorkoutTemplateServiceTest, SearchRowsFiltersByNameFragment)
{
    WorkoutTemplateQuery seen;
    EXPECT_CALL(m_rowRepo, findAll(_))
        .WillOnce(
            [&seen](const WorkoutTemplateQuery& query)
            {
                seen = query;
                return std::vector<WorkoutTemplateRow> {};
            });

    searchRows(QStringLiteral("  push  "));

    ASSERT_TRUE(seen.nameContains().has_value());
    EXPECT_EQ(seen.nameContains().value(), QStringLiteral("push"));
}

TEST_F(WorkoutTemplateServiceTest, SearchRowsWithBlankTextListsEverything)
{
    WorkoutTemplateQuery seen;
    EXPECT_CALL(m_rowRepo, findAll(_))
        .WillOnce(
            [&seen](const WorkoutTemplateQuery& query)
            {
                seen = query;
                return std::vector<WorkoutTemplateRow> {};
            });

    searchRows(QStringLiteral("   "));

    EXPECT_FALSE(seen.nameContains().has_value());
}

// --- save ---

TEST_F(WorkoutTemplateServiceTest, SaveRejectsATemplateThatFailsValidation)
{
    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = save(WorkoutTemplate(QStringLiteral("Empty")));

    EXPECT_TRUE(result.isFailure());
    EXPECT_THAT(result.error().toStdString(), ::testing::HasSubstr("at least one exercise"));
}

TEST_F(WorkoutTemplateServiceTest, SaveRejectsADuplicateNameFromAnotherTemplate)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(makeTemplate(3, QStringLiteral("Push"))));
    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = save(makeTemplate(4, QStringLiteral("Push")));

    EXPECT_TRUE(result.isFailure());
    EXPECT_THAT(result.error().toStdString(), ::testing::HasSubstr("already exists"));
}

TEST_F(WorkoutTemplateServiceTest, SaveAcceptsTheSameNameWhenUpdatingTheSameTemplate)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(makeTemplate(3, QStringLiteral("Push"))));
    EXPECT_CALL(m_repo, save(_)).WillOnce(Return(3));

    const auto result = save(makeTemplate(3, QStringLiteral("Push")));

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 3);
}

TEST_F(WorkoutTemplateServiceTest, SaveTrimsTheNameAndNormalizesPositions)
{
    WorkoutTemplate workoutTemplate(QStringLiteral("  Legs  "));
    workoutTemplate.addExercise(makeTemplateExercise(7, 2));
    workoutTemplate.addExercise(makeTemplateExercise(9, 1));

    WorkoutTemplate saved;
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const WorkoutTemplate& value)
            {
                saved = value;
                return 11;
            });

    const auto result = save(workoutTemplate);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(saved.name(), QStringLiteral("Legs"));
    EXPECT_EQ(saved.exercises()[0].position(), 0);
    EXPECT_EQ(saved.exercises()[1].position(), 1);
}

// --- saveFromWorkout ---

TEST_F(WorkoutTemplateServiceTest, SaveFromWorkoutCopiesExercisesSetsAndRest)
{
    Workout workout(QStringLiteral("Leg Day"), QDateTime::currentDateTime());
    Exercise squat = Exercise::createFromDefinition(7, QStringLiteral("Back Squat"),
                                                    ExerciseKind::Strength, 210);
    squat.setNotes(QStringLiteral("belt from set 3"));
    squat.addSet(Set(5, 100.0));
    squat.addSet(Set(3, 120.0));
    workout.addExercise(squat);

    WorkoutTemplate saved;
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const WorkoutTemplate& value)
            {
                saved = value;
                return 5;
            });

    const auto result = saveFromWorkout(workout, QStringLiteral("Leg Day Template"));

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(saved.name(), QStringLiteral("Leg Day Template"));
    ASSERT_EQ(saved.exercises().size(), 1u);
    EXPECT_EQ(saved.exercises()[0].definitionId(), 7);
    EXPECT_EQ(saved.exercises()[0].restSecondsOverride(), 210);
    EXPECT_EQ(saved.exercises()[0].notes(), QStringLiteral("belt from set 3"));
    ASSERT_EQ(saved.exercises()[0].sets().size(), 2u);
    EXPECT_EQ(saved.exercises()[0].sets()[1].repetitions(), 3);
    EXPECT_DOUBLE_EQ(saved.exercises()[0].sets()[1].weight(), 120.0);
}

TEST_F(WorkoutTemplateServiceTest, SaveFromWorkoutFallsBackToTheWorkoutName)
{
    Workout workout(QStringLiteral("Leg Day"), QDateTime::currentDateTime());
    Exercise squat = Exercise::createFromDefinition(7, QStringLiteral("Back Squat"),
                                                    ExerciseKind::Strength, 180);
    squat.addSet(Set(5, 100.0));
    workout.addExercise(squat);

    WorkoutTemplate saved;
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const WorkoutTemplate& value)
            {
                saved = value;
                return 5;
            });

    saveFromWorkout(workout, QStringLiteral("   "));

    EXPECT_EQ(saved.name(), QStringLiteral("Leg Day"));
}

TEST_F(WorkoutTemplateServiceTest, SaveFromWorkoutRejectsAnExerciseWithoutACatalogLink)
{
    Workout workout(QStringLiteral("Leg Day"), QDateTime::currentDateTime());
    Exercise adHoc
        = Exercise::createAdHoc(QStringLiteral("Sled Push"), ExerciseKind::Strength, 120);
    adHoc.addSet(Set(5, 100.0));
    workout.addExercise(adHoc);

    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = saveFromWorkout(workout, QStringLiteral("Leg Day"));

    EXPECT_TRUE(result.isFailure());
    EXPECT_THAT(result.error().toStdString(), ::testing::HasSubstr("Sled Push"));
}

// --- duplicate ---

TEST_F(WorkoutTemplateServiceTest, DuplicateClearsTheIdAndSuffixesTheName)
{
    EXPECT_CALL(m_repo, findOne(_))
        .WillOnce(Return(makeTemplate(3, QStringLiteral("Push"))))
        .WillOnce(Return(std::nullopt));

    WorkoutTemplate saved;
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const WorkoutTemplate& value)
            {
                saved = value;
                return 8;
            });

    const auto result = duplicate(3, QString());

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 8);
    EXPECT_EQ(saved.id(), -1);
    EXPECT_EQ(saved.name(), QStringLiteral("Push (copy)"));
    EXPECT_EQ(saved.exercises().size(), 1u);
}

TEST_F(WorkoutTemplateServiceTest, DuplicateFailsForAMissingTemplate)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = duplicate(42, QString());

    EXPECT_TRUE(result.isFailure());
    EXPECT_THAT(result.error().toStdString(), ::testing::HasSubstr("does not exist"));
}

// --- remove ---

TEST_F(WorkoutTemplateServiceTest, RemoveReturnsFalseForAMissingTemplate)
{
    EXPECT_CALL(m_repo, exists(_)).WillOnce(Return(false));
    EXPECT_CALL(m_repo, remove(_)).Times(0);

    const auto result = remove(42);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value());
}

TEST_F(WorkoutTemplateServiceTest, RemoveDeletesAnExistingTemplate)
{
    EXPECT_CALL(m_repo, exists(_)).WillOnce(Return(true));
    EXPECT_CALL(m_repo, remove(_)).WillOnce(Return(true));

    const auto result = remove(3);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
}

// --- instantiate ---

TEST_F(WorkoutTemplateServiceTest, InstantiateBuildsAPlannedWorkoutFromTheCatalog)
{
    WorkoutTemplate source(QStringLiteral("Push"));
    source.setId(3);
    source.addExercise(makeTemplateExercise(7, 3));
    source.addExercise(makeTemplateExercise(9, 1));

    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(source));

    const QDateTime planned = QDateTime::currentDateTime().addDays(2);
    const auto result = instantiate(3, planned);

    ASSERT_TRUE(result.isSuccess());
    const Workout& workout = result.value();
    EXPECT_EQ(workout.name(), QStringLiteral("Push"));
    EXPECT_EQ(workout.plannedTime(), planned);
    EXPECT_EQ(workout.status(), WorkoutStatus::Planned);
    ASSERT_EQ(workout.exercises().size(), 2u);
    EXPECT_EQ(workout.exercises()[0].name(), QStringLiteral("Back Squat"));
    EXPECT_EQ(workout.exercises()[0].kind(), ExerciseKind::Strength);
    EXPECT_EQ(workout.exercises()[0].sets().size(), 3u);
    EXPECT_EQ(workout.exercises()[1].name(), QStringLiteral("Plank"));
    EXPECT_EQ(workout.exercises()[1].position(), 1);
}

TEST_F(WorkoutTemplateServiceTest, InstantiatePrefersTheTemplateRestOverrideOverTheCatalogDefault)
{
    WorkoutTemplate source(QStringLiteral("Push"));
    source.setId(3);
    TemplateExercise exercise = makeTemplateExercise(7, 1);
    exercise.setRestSecondsOverride(45);
    source.addExercise(exercise);

    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(source));

    const auto result = instantiate(3, QDateTime::currentDateTime());

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value().exercises()[0].restSeconds(), 45);
}

TEST_F(WorkoutTemplateServiceTest, InstantiateFallsBackToTheCatalogRestWhenNotOverridden)
{
    WorkoutTemplate source(QStringLiteral("Push"));
    source.setId(3);
    source.addExercise(makeTemplateExercise(7, 1));

    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(source));

    const auto result = instantiate(3, QDateTime::currentDateTime());

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value().exercises()[0].restSeconds(), 180);
}

TEST_F(WorkoutTemplateServiceTest, InstantiateFailsWhenADefinitionIsMissingFromTheCatalog)
{
    WorkoutTemplate source(QStringLiteral("Push"));
    source.setId(3);
    source.addExercise(makeTemplateExercise(404, 1));

    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(source));

    const auto result = instantiate(3, QDateTime::currentDateTime());

    EXPECT_TRUE(result.isFailure());
    EXPECT_THAT(result.error().toStdString(), ::testing::HasSubstr("404"));
}

TEST_F(WorkoutTemplateServiceTest, InstantiateFailsForAMissingTemplate)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));

    const auto result = instantiate(42, QDateTime::currentDateTime());

    EXPECT_TRUE(result.isFailure());
    EXPECT_THAT(result.error().toStdString(), ::testing::HasSubstr("does not exist"));
}
