#include "modules/exercisecatalog/application/exercisecatalogservice.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionquery.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionrepository.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class MockExerciseDefinitionRepository : public ExerciseDefinitionRepository
{
public:
    MOCK_METHOD(std::vector<ExerciseDefinition>, findAll, (const ExerciseDefinitionQuery& query),
                (const, override));
    MOCK_METHOD(std::optional<ExerciseDefinition>, findOne, (const ExerciseDefinitionQuery& query),
                (const, override));
    MOCK_METHOD(int, save, (const ExerciseDefinition& definition), (override));
    MOCK_METHOD(bool, remove, (const ExerciseDefinitionQuery& query), (override));
    MOCK_METHOD(int, count, (const ExerciseDefinitionQuery& query), (const, override));
    MOCK_METHOD(bool, exists, (const ExerciseDefinitionQuery& query), (const, override));
};

class ExerciseCatalogServiceTest : public ::testing::Test
{
protected:
    void SetUp() override { m_service = std::make_unique<ExerciseCatalogService>(m_repo, nullptr); }

    static ExerciseDefinition builtIn(int id, const QString& slug, const QString& name)
    {
        ExerciseDefinition definition(slug, name, ExerciseKind::Strength);
        definition.setId(id);
        definition.addMuscle({ Muscle::Chest, MuscleRole::Primary });
        return definition;
    }

    static ExerciseDefinition custom(int id, const QString& slug, const QString& name)
    {
        ExerciseDefinition definition = builtIn(id, slug, name);
        definition.setOrigin(CatalogOrigin::Custom);
        return definition;
    }

    Result<std::vector<ExerciseDefinition>> search(const ExerciseDefinitionQuery& query)
    {
        return m_service->searchCore(query);
    }
    Result<int> saveCustom(const ExerciseDefinition& definition)
    {
        return m_service->saveCustomCore(definition);
    }
    Result<std::optional<int>> match(const QString& name) { return m_service->matchCore(name); }
    Result<int> matchOrImport(const QString& name, ExerciseKind kind)
    {
        return m_service->matchOrImportCore(name, kind);
    }
    Result<bool> archive(int id) { return m_service->archiveCore(id); }
    Result<bool> restore(int id) { return m_service->restoreCore(id); }
    Result<bool> remove(int id) { return m_service->removeCore(id); }

    MockExerciseDefinitionRepository m_repo;
    std::unique_ptr<ExerciseCatalogService> m_service;
};

TEST_F(ExerciseCatalogServiceTest, SearchHidesArchivedExercisesByDefault)
{
    ExerciseDefinitionQuery seen;
    EXPECT_CALL(m_repo, findAll(_))
        .WillOnce(
            [&seen](const ExerciseDefinitionQuery& query)
            {
                seen = query;
                return std::vector<ExerciseDefinition> {};
            });

    search(ExerciseDefinitionQuery());

    ASSERT_TRUE(seen.archived().has_value());
    EXPECT_FALSE(seen.archived().value());
}

TEST_F(ExerciseCatalogServiceTest, SearchSortsByNameByDefault)
{
    ExerciseDefinitionQuery seen;
    EXPECT_CALL(m_repo, findAll(_))
        .WillOnce(
            [&seen](const ExerciseDefinitionQuery& query)
            {
                seen = query;
                return std::vector<ExerciseDefinition> {};
            });

    search(ExerciseDefinitionQuery());

    ASSERT_TRUE(seen.orderByNameDirection().has_value());
    EXPECT_EQ(seen.orderByNameDirection().value(), SortDirection::Ascending);
}

TEST_F(ExerciseCatalogServiceTest, SearchKeepsAnExplicitArchivedFilter)
{
    ExerciseDefinitionQuery seen;
    EXPECT_CALL(m_repo, findAll(_))
        .WillOnce(
            [&seen](const ExerciseDefinitionQuery& query)
            {
                seen = query;
                return std::vector<ExerciseDefinition> {};
            });

    search(ExerciseDefinitionQuery().whereArchived(true));

    ASSERT_TRUE(seen.archived().has_value());
    EXPECT_TRUE(seen.archived().value());
}

TEST_F(ExerciseCatalogServiceTest, SavingACustomExerciseDerivesTheSlugFromTheName)
{
    ExerciseDefinition definition;
    definition.setName("Landmine Press");
    definition.addMuscle({ Muscle::FrontDelts, MuscleRole::Primary });

    ExerciseDefinition saved;
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const ExerciseDefinition& value)
            {
                saved = value;
                return 7;
            });

    const auto result = saveCustom(definition);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 7);
    EXPECT_EQ(saved.slug(), "landmine-press");
    EXPECT_EQ(saved.origin(), CatalogOrigin::Custom);
}

TEST_F(ExerciseCatalogServiceTest, AnInvalidCustomExerciseIsRejectedBeforeReachingTheRepository)
{
    ExerciseDefinition definition;
    definition.setName("Mystery Lift");

    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = saveCustom(definition);

    ASSERT_TRUE(result.isFailure());
    EXPECT_TRUE(result.error().contains("primary"));
}

TEST_F(ExerciseCatalogServiceTest, ACustomExerciseCannotOverwriteABuiltIn)
{
    ExerciseDefinition definition("bench-press", "Bench Press", ExerciseKind::Strength);
    definition.addMuscle({ Muscle::Chest, MuscleRole::Primary });

    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(builtIn(1, "bench-press", "Bench Press")));
    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = saveCustom(definition);

    ASSERT_TRUE(result.isFailure());
    EXPECT_TRUE(result.error().contains("built-in"));
}

TEST_F(ExerciseCatalogServiceTest, ResavingAnExistingCustomExerciseKeepsItsId)
{
    ExerciseDefinition definition("my-lift", "My Lift", ExerciseKind::Strength);
    definition.addMuscle({ Muscle::Chest, MuscleRole::Primary });

    ExerciseDefinition saved;
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(custom(12, "my-lift", "My Lift")));
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const ExerciseDefinition& value)
            {
                saved = value;
                return value.id();
            });

    const auto result = saveCustom(definition);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(saved.id(), 12);
}

TEST_F(ExerciseCatalogServiceTest, MatchingLooksOnlyAtActiveExercises)
{
    ExerciseDefinitionQuery seen;
    EXPECT_CALL(m_repo, findAll(_))
        .WillOnce(
            [&seen](const ExerciseDefinitionQuery& query)
            {
                seen = query;
                return std::vector<ExerciseDefinition> {};
            });

    match("Bench Press");

    ASSERT_TRUE(seen.archived().has_value());
    EXPECT_FALSE(seen.archived().value());
}

TEST_F(ExerciseCatalogServiceTest, MatchingFindsAKnownExercise)
{
    EXPECT_CALL(m_repo, findAll(_))
        .WillOnce(Return(std::vector<ExerciseDefinition> {
            builtIn(3, "bench-press", "Bench Press"), builtIn(4, "back-squat", "Back Squat") }));

    const auto result = match("bench press");

    ASSERT_TRUE(result.isSuccess());
    ASSERT_TRUE(result.value().has_value());
    EXPECT_EQ(result.value().value(), 3);
}

TEST_F(ExerciseCatalogServiceTest, MatchingAnUnknownNameYieldsNothing)
{
    EXPECT_CALL(m_repo, findAll(_))
        .WillOnce(
            Return(std::vector<ExerciseDefinition> { builtIn(3, "bench-press", "Bench Press") }));

    const auto result = match("Zercher Squat");

    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value().has_value());
}

TEST_F(ExerciseCatalogServiceTest, ImportReusesAMatchInsteadOfCreatingADuplicate)
{
    EXPECT_CALL(m_repo, findAll(_))
        .WillOnce(
            Return(std::vector<ExerciseDefinition> { builtIn(3, "bench-press", "Bench Press") }));
    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = matchOrImport("Bench Press", ExerciseKind::Strength);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 3);
}

TEST_F(ExerciseCatalogServiceTest, ImportCreatesAnImportedDefinitionForAnUnknownName)
{
    EXPECT_CALL(m_repo, findAll(_)).WillOnce(Return(std::vector<ExerciseDefinition> {}));
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));

    ExerciseDefinition saved;
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const ExerciseDefinition& value)
            {
                saved = value;
                return 21;
            });

    const auto result = matchOrImport("Zercher Squat", ExerciseKind::Strength);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 21);
    EXPECT_EQ(saved.name(), "Zercher Squat");
    EXPECT_EQ(saved.slug(), "zercher-squat");
    EXPECT_EQ(saved.origin(), CatalogOrigin::Imported);
}

TEST_F(ExerciseCatalogServiceTest, ImportNeverOverwritesADefinitionThatSharesTheSlug)
{
    ExerciseDefinitionQuery seen;
    EXPECT_CALL(m_repo, findAll(_)).WillOnce(Return(std::vector<ExerciseDefinition> {}));
    EXPECT_CALL(m_repo, findOne(_))
        .WillOnce(
            [&seen](const ExerciseDefinitionQuery& query)
            {
                seen = query;
                return builtIn(9, "bench-press", "Bench Press");
            });
    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = matchOrImport("Bench  Press", ExerciseKind::Strength);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 9);
    ASSERT_TRUE(seen.slug().has_value());
    EXPECT_EQ(seen.slug().value(), "bench-press");
}

TEST_F(ExerciseCatalogServiceTest, ImportRefusesANamelessExercise)
{
    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = matchOrImport("   ", ExerciseKind::Strength);

    ASSERT_TRUE(result.isFailure());
}

TEST_F(ExerciseCatalogServiceTest, ArchivingFlagsTheExerciseWithoutDeletingIt)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(builtIn(5, "bench-press", "Bench Press")));

    ExerciseDefinition saved;
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const ExerciseDefinition& value)
            {
                saved = value;
                return value.id();
            });
    EXPECT_CALL(m_repo, remove(_)).Times(0);

    const auto result = archive(5);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
    EXPECT_TRUE(saved.isArchived());
}

TEST_F(ExerciseCatalogServiceTest, RestoringClearsTheArchivedFlag)
{
    ExerciseDefinition archived = builtIn(5, "bench-press", "Bench Press");
    archived.setArchived(true);
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(archived));

    ExerciseDefinition saved;
    EXPECT_CALL(m_repo, save(_))
        .WillOnce(
            [&saved](const ExerciseDefinition& value)
            {
                saved = value;
                return value.id();
            });

    const auto result = restore(5);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(saved.isArchived());
}

TEST_F(ExerciseCatalogServiceTest, ArchivingAnUnknownExerciseReportsNoChange)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(m_repo, save(_)).Times(0);

    const auto result = archive(404);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value());
}

TEST_F(ExerciseCatalogServiceTest, ABuiltInExerciseCannotBeDeleted)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(builtIn(5, "bench-press", "Bench Press")));
    EXPECT_CALL(m_repo, remove(_)).Times(0);

    const auto result = remove(5);

    ASSERT_TRUE(result.isFailure());
    EXPECT_TRUE(result.error().contains("archived"));
}

TEST_F(ExerciseCatalogServiceTest, ACustomExerciseCanBeDeleted)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(custom(6, "my-lift", "My Lift")));
    EXPECT_CALL(m_repo, remove(_)).WillOnce(Return(true));

    const auto result = remove(6);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
}

TEST_F(ExerciseCatalogServiceTest, DeletingAnUnknownExerciseReportsNoChange)
{
    EXPECT_CALL(m_repo, findOne(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(m_repo, remove(_)).Times(0);

    const auto result = remove(404);

    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value());
}
