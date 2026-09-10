#include <gtest/gtest.h>

#include "application/workout/workoutservice.h"
#include "domain/exercisecatalog/exercisedefinition.h"
#include "domain/workout/exercise.h"
#include "domain/workout/performedsets.h"
#include "domain/workout/workout.h"
#include "fixtures/test_data.h"
#include "testapplication.h"
#include "ui/models/exercisedefinitionmodel.h"
#include "ui/models/exercisemodel.h"
#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"

namespace
{

ExerciseDefinition makeDefinition(const QString& slug, const QString& name, ExerciseKind kind,
                                  SetMetric metric, LoadType loadType, int restSeconds)
{
    ExerciseDefinition definition(slug, name, kind);
    definition.setDefaultMetric(metric);
    definition.setDefaultLoadType(loadType);
    definition.setDefaultRestSeconds(restSeconds);
    definition.addMuscle({ Muscle::Quads, MuscleRole::Primary });
    return definition;
}

}

class ActiveWorkoutAddExerciseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_app.seedDefinition(makeDefinition(QStringLiteral("back-squat"),
                                            QStringLiteral("Back Squat"), ExerciseKind::Strength,
                                            SetMetric::Reps, LoadType::External, 180));
        m_app.seedDefinition(makeDefinition(QStringLiteral("plank"), QStringLiteral("Plank"),
                                            ExerciseKind::Interval, SetMetric::Duration,
                                            LoadType::None, 60));
        m_app.drain();

        catalog().load();
        m_app.drain();
    }

    void TearDown() override
    {
        if (active().currentWorkout() != nullptr)
            active().endWorkout();
        m_app.drain();
    }

    ActiveWorkoutViewModel& active() { return m_app.activeWorkoutViewModel(); }
    ExerciseCatalogViewModel& catalog() { return m_app.exerciseCatalogViewModel(); }
    PlannedWorkoutViewModel& planned() { return m_app.plannedWorkoutViewModel(); }

    ExerciseDefinitionModel* definition(const QString& name)
    {
        for (auto* model : catalog().exercises())
        {
            if (model->name() == name)
                return model;
        }
        return nullptr;
    }

    int definitionId(const QString& name)
    {
        ExerciseDefinitionModel* model = definition(name);
        return model == nullptr ? -1 : model->entity().id();
    }

    void startPlannedWorkout()
    {
        planned().importFromJson(TestData::THREE_WORKOUTS_JSON);
        planned().loadAll();
        m_app.drain();

        WorkoutModel* target = nullptr;
        for (auto* workout : planned().workouts())
        {
            if (workout->name() == QStringLiteral("Push Day"))
                target = workout;
        }

        ASSERT_NE(target, nullptr);
        active().startWorkout(target);
        m_app.drain();
    }

    void addExercise(const QString& name)
    {
        ExerciseDefinitionModel* model = definition(name);
        ASSERT_NE(model, nullptr) << name.toStdString();
        active().addExercise(model);
        m_app.drain();
    }

    static Set liftedSet(int repetitions, double weight)
    {
        Set set(repetitions, weight);
        set.setCompleted(true);
        return set;
    }

    void seedLinkedHistory(const QString& name, std::initializer_list<Set> sets, const QDate& date)
    {
        Exercise exercise
            = Exercise::createFromDefinition(definitionId(name), name, ExerciseKind::Strength, 180);
        for (const Set& set : sets)
            exercise.addSet(set);

        Workout past(QStringLiteral("Past session"), QDateTime(date, QTime(18, 0, 0)));
        past.setStartedTime(QDateTime(date, QTime(18, 0, 0)));
        past.setEndedTime(QDateTime(date, QTime(19, 0, 0)));
        past.addExercise(exercise);

        m_app.workoutService()
            .importHistory(std::vector<Workout> { past })
            .warnOnError("seed the workout history");
        m_app.drain();
    }

    ExerciseModel* lastExercise() { return active().currentWorkout()->exercises().last(); }

    Workout storedWorkout(int workoutId)
    {
        std::optional<Workout> found;
        m_app.workoutService()
            .findWorkout(workoutId)
            .then(&active(), [&found](std::optional<Workout> result) { found = result; })
            .warnOnError("reload the finished workout");
        m_app.drain();

        return found.value_or(Workout());
    }

    static const Exercise* exerciseNamed(const Workout& workout, const QString& name)
    {
        for (const Exercise& exercise : workout.exercises())
        {
            if (exercise.name() == name)
                return &exercise;
        }
        return nullptr;
    }

    TestApplication m_app;
};

TEST_F(ActiveWorkoutAddExerciseTest, ACatalogExerciseLandsAtTheEndOfTheActiveWorkout)
{
    startPlannedWorkout();
    const int before = active().currentWorkout()->exercises().size();

    addExercise(QStringLiteral("Back Squat"));

    ASSERT_EQ(active().currentWorkout()->exercises().size(), before + 1);
    EXPECT_EQ(lastExercise()->name(), QStringLiteral("Back Squat"));
}

TEST_F(ActiveWorkoutAddExerciseTest, TheAddedExerciseArrivesWithOneOpenSet)
{
    startPlannedWorkout();
    const int setsBefore = active().totalSetCount();

    addExercise(QStringLiteral("Back Squat"));

    ASSERT_EQ(lastExercise()->sets().size(), 1);
    EXPECT_FALSE(lastExercise()->sets().first()->completed());
    EXPECT_EQ(active().totalSetCount(), setsBefore + 1);
}

TEST_F(ActiveWorkoutAddExerciseTest, TheAddedSetCanBeTickedOff)
{
    startPlannedWorkout();
    addExercise(QStringLiteral("Back Squat"));

    const int doneBefore = active().completedSetCount();
    active().toggleSetCompleted(lastExercise()->sets().first());
    m_app.drain();

    EXPECT_TRUE(lastExercise()->sets().first()->completed());
    EXPECT_EQ(active().completedSetCount(), doneBefore + 1);
}

TEST_F(ActiveWorkoutAddExerciseTest, AnExerciseWithHistoryEntersWithLastTimesWeight)
{
    seedLinkedHistory(QStringLiteral("Back Squat"), { liftedSet(10, 60.0), liftedSet(5, 82.5) },
                      QDate(2024, 12, 20));
    startPlannedWorkout();

    addExercise(QStringLiteral("Back Squat"));

    EXPECT_EQ(lastExercise()->sets().first()->repetitions(), 5);
    EXPECT_DOUBLE_EQ(lastExercise()->sets().first()->weight(), 82.5);
    EXPECT_FALSE(lastExercise()->sets().first()->completed());
}

TEST_F(ActiveWorkoutAddExerciseTest, AnExerciseWithoutHistoryKeepsTheDefaultSeed)
{
    startPlannedWorkout();

    addExercise(QStringLiteral("Plank"));

    EXPECT_EQ(lastExercise()->sets().first()->durationSeconds(), 30);
    EXPECT_DOUBLE_EQ(lastExercise()->sets().first()->weight(), 0.0);
}

TEST_F(ActiveWorkoutAddExerciseTest, TheHistoricalWeightSurvivesIntoTheStoredSession)
{
    seedLinkedHistory(QStringLiteral("Back Squat"), { liftedSet(5, 82.5) }, QDate(2024, 12, 20));
    startPlannedWorkout();
    addExercise(QStringLiteral("Back Squat"));

    const int workoutId = active().currentWorkout()->id();
    active().toggleSetCompleted(lastExercise()->sets().first());
    active().endWorkout();
    m_app.drain();

    const Workout stored = storedWorkout(workoutId);
    const Exercise* added = exerciseNamed(stored, QStringLiteral("Back Squat"));
    ASSERT_NE(added, nullptr);
    ASSERT_EQ(added->sets().size(), 1u);
    EXPECT_DOUBLE_EQ(added->sets().front().weight(), 82.5);
}

TEST_F(ActiveWorkoutAddExerciseTest, TheAddedExerciseIsTheLastOneInTheStoredSession)
{
    startPlannedWorkout();
    addExercise(QStringLiteral("Back Squat"));

    const int workoutId = active().currentWorkout()->id();
    active().toggleSetCompleted(lastExercise()->sets().first());
    active().endWorkout();
    m_app.drain();

    const Workout stored = storedWorkout(workoutId);
    ASSERT_FALSE(stored.exercises().empty());
    EXPECT_EQ(stored.exercises().back().name(), QStringLiteral("Back Squat"));
}

TEST_F(ActiveWorkoutAddExerciseTest, TickingOnlyTheAddedExerciseDropsThePlannedOneFromTheRecord)
{
    startPlannedWorkout();
    addExercise(QStringLiteral("Back Squat"));

    const int workoutId = active().currentWorkout()->id();
    active().toggleSetCompleted(lastExercise()->sets().first());
    active().endWorkout();
    m_app.drain();

    const Workout stored = storedWorkout(workoutId);
    ASSERT_TRUE(completionFlagsAreMeaningful(stored));

    const Exercise* added = exerciseNamed(stored, QStringLiteral("Back Squat"));
    ASSERT_NE(added, nullptr);
    EXPECT_EQ(exerciseNamed(stored, QStringLiteral("Bench Press")), nullptr);

    ASSERT_EQ(added->sets().size(), 1u);
    EXPECT_TRUE(added->sets().front().completed());
    EXPECT_TRUE(wasPerformed(*added, true));
}

TEST_F(ActiveWorkoutAddExerciseTest, ASessionWithoutASingleTickKeepsTheAmnestyAfterAnAddition)
{
    startPlannedWorkout();
    addExercise(QStringLiteral("Back Squat"));

    const int workoutId = active().currentWorkout()->id();
    active().endWorkout();
    m_app.drain();

    const Workout stored = storedWorkout(workoutId);
    EXPECT_FALSE(completionFlagsAreMeaningful(stored));

    const Exercise* added = exerciseNamed(stored, QStringLiteral("Back Squat"));
    ASSERT_NE(added, nullptr);
    EXPECT_FALSE(added->sets().front().completed());
    EXPECT_TRUE(wasPerformed(*added, completionFlagsAreMeaningful(stored)));
}

TEST_F(ActiveWorkoutAddExerciseTest, AddingWithoutAnActiveWorkoutChangesNothing)
{
    if (active().currentWorkout() != nullptr)
        active().discardWorkout();
    m_app.drain();
    ASSERT_EQ(active().currentWorkout(), nullptr);

    active().addExercise(definition(QStringLiteral("Back Squat")));
    m_app.drain();

    EXPECT_EQ(active().currentWorkout(), nullptr);
}

TEST_F(ActiveWorkoutAddExerciseTest, AddingNothingChangesNothing)
{
    startPlannedWorkout();
    const int before = active().currentWorkout()->exercises().size();

    active().addExercise(nullptr);
    m_app.drain();

    EXPECT_EQ(active().currentWorkout()->exercises().size(), before);
}
