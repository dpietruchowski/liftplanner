#include "application/exercisecatalog/exercisecatalogservice.h"
#include "application/workout/workoutservice.h"
#include "domain/exercisecatalog/exercisedefinition.h"
#include "domain/workout/workout.h"
#include "domain/workout/workoutstatus.h"
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

    int definitionId(const QString& name)
    {
        viewModel().load();
        m_app.drain();
        for (const auto* model : viewModel().exercises())
        {
            if (model->name() == name)
                return model->definitionId();
        }
        return -1;
    }

    static Exercise performed(Exercise exercise)
    {
        Set set(5, 100.0);
        set.setCompleted(true);
        exercise.addSet(set);
        return exercise;
    }

    Exercise linked(const QString& name)
    {
        return performed(
            Exercise::createFromDefinition(definitionId(name), name, ExerciseKind::Strength, 120));
    }

    void seedSession(std::initializer_list<Exercise> exercises, const QDate& date,
                     WorkoutStatus status = WorkoutStatus::Ended)
    {
        Workout session(QStringLiteral("Session"), QDateTime(date, QTime(18, 0, 0)));
        session.setStartedTime(QDateTime(date, QTime(18, 0, 0)));
        session.setEndedTime(QDateTime(date, QTime(19, 0, 0)));
        for (const Exercise& exercise : exercises)
            session.addExercise(exercise);

        if (status == WorkoutStatus::Ended)
        {
            m_app.workoutService()
                .importHistory(std::vector<Workout> { session })
                .warnOnError("seed a session");
        }
        else
        {
            m_app.workoutService()
                .importPlannedWorkouts(std::vector<Workout> { session })
                .warnOnError("seed a planned session");
        }
        m_app.drain();
    }

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

TEST_F(ExerciseCatalogTest, RefreshPutsRecentlyDoneExercisesFirstNewestFirst)
{
    seedSession({ linked(QStringLiteral("Push Up")) }, QDate(2024, 12, 20));
    seedSession({ linked(QStringLiteral("Front Squat")), linked(QStringLiteral("Push Up")) },
                QDate(2024, 12, 28));

    viewModel().refresh();
    m_app.drain();

    EXPECT_EQ(viewModel().recentCount(), 2);
    EXPECT_EQ(loadedNames(),
              QStringList({ QStringLiteral("Front Squat"), QStringLiteral("Push Up"),
                            QStringLiteral("Back Squat"), QStringLiteral("Bench Press"),
                            QStringLiteral("Treadmill Run") }));
    EXPECT_EQ(viewModel().exercises().first()->lastPerformed().date(), QDate(2024, 12, 28));
    EXPECT_FALSE(viewModel().exercises().at(2)->isRecent());
}

TEST_F(ExerciseCatalogTest, AnExerciseWithoutADefinitionMatchesTheCatalogByName)
{
    seedSession({ performed(Exercise::createAdHoc(QStringLiteral("bench press"),
                                                  ExerciseKind::Strength, 120)) },
                QDate(2024, 12, 28));

    viewModel().refresh();
    m_app.drain();

    EXPECT_EQ(viewModel().recentCount(), 1);
    EXPECT_EQ(loadedNames().first(), QStringLiteral("Bench Press"));
}

TEST_F(ExerciseCatalogTest, PlannedAndUntouchedExercisesAreNotRecent)
{
    seedSession({ linked(QStringLiteral("Push Up")) }, QDate(2025, 1, 5), WorkoutStatus::Planned);
    seedSession({ Exercise::createFromDefinition(definitionId(QStringLiteral("Treadmill Run")),
                                                 QStringLiteral("Treadmill Run"),
                                                 ExerciseKind::Cardio, 120) },
                QDate(2024, 12, 28));

    viewModel().refresh();
    m_app.drain();

    EXPECT_EQ(viewModel().recentCount(), 0);
    EXPECT_EQ(loadedNames().first(), QStringLiteral("Back Squat"));
}

TEST_F(ExerciseCatalogTest, RecentExercisesStayFirstWhileSearching)
{
    seedSession({ linked(QStringLiteral("Front Squat")) }, QDate(2024, 12, 28));

    viewModel().refresh();
    m_app.drain();
    viewModel().setSearchText(QStringLiteral("squat"));
    m_app.drain();

    EXPECT_EQ(viewModel().recentCount(), 1);
    EXPECT_EQ(loadedNames(),
              QStringList({ QStringLiteral("Front Squat"), QStringLiteral("Back Squat") }));
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
