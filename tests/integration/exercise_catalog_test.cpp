#include "application/exercisecatalog/exercisecatalogservice.h"
#include "domain/exercisecatalog/exercisedefinition.h"
#include "testapplication.h"
#include "ui/models/exercisedefinitionmodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"

#include <QSignalSpy>
#include <gtest/gtest.h>

namespace
{

ExerciseDefinition makeDefinition(const QString& slug, const QString& name, ExerciseKind kind,
                                  Equipment equipment, Muscle primaryMuscle)
{
    ExerciseDefinition definition(slug, name, kind);
    definition.setEquipment(equipment);
    definition.addMuscle({ primaryMuscle, MuscleRole::Primary });
    definition.setDefaultRestSeconds(120);
    return definition;
}

}  // namespace

class ExerciseCatalogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_app.seedDefinition(makeDefinition(QStringLiteral("back-squat"),
                                            QStringLiteral("Back Squat"), ExerciseKind::Strength,
                                            Equipment::Barbell, Muscle::Quads));
        m_app.seedDefinition(makeDefinition(QStringLiteral("front-squat"),
                                            QStringLiteral("Front Squat"), ExerciseKind::Strength,
                                            Equipment::Barbell, Muscle::Quads));
        m_app.seedDefinition(makeDefinition(QStringLiteral("bench-press"),
                                            QStringLiteral("Bench Press"), ExerciseKind::Strength,
                                            Equipment::Barbell, Muscle::Chest));
        m_app.seedDefinition(makeDefinition(QStringLiteral("push-up"), QStringLiteral("Push Up"),
                                            ExerciseKind::Bodyweight, Equipment::Bodyweight,
                                            Muscle::Chest));
        m_app.seedDefinition(makeDefinition(QStringLiteral("treadmill-run"),
                                            QStringLiteral("Treadmill Run"), ExerciseKind::Cardio,
                                            Equipment::Treadmill, Muscle::Quads));
        m_app.drain();
    }

    QStringList loadedNames()
    {
        QStringList names;
        for (const auto* model : viewModel().exercises())
            names.append(model->name());
        return names;
    }

    ExerciseCatalogViewModel& viewModel() { return m_app.exerciseCatalogViewModel(); }

    TestApplication m_app;
};

TEST_F(ExerciseCatalogTest, LoadExposesTheWholeCatalogSortedByName)
{
    viewModel().load();
    m_app.drain();

    EXPECT_EQ(viewModel().count(), 5);
    EXPECT_EQ(loadedNames(),
              QStringList({ QStringLiteral("Back Squat"), QStringLiteral("Bench Press"),
                            QStringLiteral("Front Squat"), QStringLiteral("Push Up"),
                            QStringLiteral("Treadmill Run") }));
}

TEST_F(ExerciseCatalogTest, SearchTextNarrowsTheListAndReloadsOnItsOwn)
{
    viewModel().load();
    m_app.drain();

    viewModel().setSearchText(QStringLiteral("squat"));
    m_app.drain();

    EXPECT_EQ(loadedNames(),
              QStringList({ QStringLiteral("Back Squat"), QStringLiteral("Front Squat") }));
}

TEST_F(ExerciseCatalogTest, MuscleFilterKeepsOnlyExercisesTargetingThatMuscle)
{
    viewModel().setMuscle(QStringLiteral("chest"));
    m_app.drain();

    EXPECT_EQ(loadedNames(),
              QStringList({ QStringLiteral("Bench Press"), QStringLiteral("Push Up") }));
}

TEST_F(ExerciseCatalogTest, EquipmentFilterKeepsOnlyMatchingEquipment)
{
    viewModel().setEquipment(QStringLiteral("bodyweight"));
    m_app.drain();

    EXPECT_EQ(loadedNames(), QStringList({ QStringLiteral("Push Up") }));
}

TEST_F(ExerciseCatalogTest, KindFilterKeepsOnlyMatchingKind)
{
    viewModel().setKind(QStringLiteral("cardio"));
    m_app.drain();

    EXPECT_EQ(loadedNames(), QStringList({ QStringLiteral("Treadmill Run") }));
}

TEST_F(ExerciseCatalogTest, FiltersCombineInsteadOfReplacingEachOther)
{
    viewModel().setMuscle(QStringLiteral("chest"));
    m_app.drain();
    viewModel().setEquipment(QStringLiteral("barbell"));
    m_app.drain();

    EXPECT_EQ(loadedNames(), QStringList({ QStringLiteral("Bench Press") }));
}

TEST_F(ExerciseCatalogTest, ClearFiltersRestoresTheFullList)
{
    viewModel().setMuscle(QStringLiteral("chest"));
    viewModel().setSearchText(QStringLiteral("bench"));
    m_app.drain();
    ASSERT_TRUE(viewModel().isFiltered());

    viewModel().clearFilters();
    m_app.drain();

    EXPECT_FALSE(viewModel().isFiltered());
    EXPECT_EQ(viewModel().count(), 5);
}

TEST_F(ExerciseCatalogTest, SettingTheSameFilterTwiceDoesNotReload)
{
    viewModel().setSearchText(QStringLiteral("squat"));
    m_app.drain();

    QSignalSpy spy(&viewModel(), &ExerciseCatalogViewModel::exercisesChanged);
    viewModel().setSearchText(QStringLiteral("  squat  "));
    m_app.drain();

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ExerciseCatalogTest, ArchivedExercisesAreHiddenFromTheCatalog)
{
    viewModel().setSearchText(QStringLiteral("push up"));
    m_app.drain();
    ASSERT_EQ(viewModel().count(), 1);
    const int pushUpId = viewModel().exercises().first()->definitionId();

    viewModel().clearFilters();
    m_app.drain();

    bool archived = false;
    m_app.exerciseCatalogService()
        .archive(pushUpId)
        .then(&viewModel(), [&archived](bool result) { archived = result; })
        .warnOnError("archive the exercise");
    m_app.drain();
    ASSERT_TRUE(archived);

    viewModel().load();
    m_app.drain();

    EXPECT_EQ(viewModel().count(), 4);
    EXPECT_FALSE(loadedNames().contains(QStringLiteral("Push Up")));
}

TEST_F(ExerciseCatalogTest, ModelExposesTheFieldsTheExercisePickerNeeds)
{
    viewModel().setSearchText(QStringLiteral("push up"));
    m_app.drain();

    ASSERT_EQ(viewModel().count(), 1);
    const ExerciseDefinitionModel* model = viewModel().exercises().first();
    EXPECT_EQ(model->name(), QStringLiteral("Push Up"));
    EXPECT_EQ(model->slug(), QStringLiteral("push-up"));
    EXPECT_EQ(model->kind(), QStringLiteral("bodyweight"));
    EXPECT_EQ(model->equipment(), QStringLiteral("bodyweight"));
    EXPECT_EQ(model->defaultRestSeconds(), 120);
    EXPECT_EQ(model->primaryMuscles(), QStringList({ QStringLiteral("chest") }));
    EXPECT_EQ(model->regions(), QStringList({ QStringLiteral("chest") }));
    EXPECT_FALSE(model->isArchived());
    EXPECT_GT(model->definitionId(), 0);
}

TEST_F(ExerciseCatalogTest, FindByIdReturnsTheLoadedModel)
{
    viewModel().load();
    m_app.drain();

    const int id = viewModel().exercises().first()->definitionId();

    ASSERT_NE(viewModel().findById(id), nullptr);
    EXPECT_EQ(viewModel().findById(id)->definitionId(), id);
    EXPECT_EQ(viewModel().findById(9999), nullptr);
}
