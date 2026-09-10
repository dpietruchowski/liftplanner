#include "application/workout/workoutservice.h"
#include "application/workout/workouttemplateservice.h"
#include "domain/exercisecatalog/exercisedefinition.h"
#include "testapplication.h"
#include "ui/models/exercisedefinitionmodel.h"
#include "ui/models/exercisemodel.h"
#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouteditorviewmodel.h"

#include <QSignalSpy>
#include <gtest/gtest.h>

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

}  // namespace

class WorkoutEditorTest : public ::testing::Test
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
        m_app.seedDefinition(makeDefinition(QStringLiteral("row-2k"), QStringLiteral("Row 2k"),
                                            ExerciseKind::Cardio, SetMetric::Distance,
                                            LoadType::None, 120));
        m_app.drain();

        catalog().load();
        m_app.drain();
    }

    ExerciseCatalogViewModel& catalog() { return m_app.exerciseCatalogViewModel(); }
    WorkoutEditorViewModel& editor() { return m_app.workoutEditorViewModel(); }
    PlannedWorkoutViewModel& planned() { return m_app.plannedWorkoutViewModel(); }

    QList<WorkoutModel*> reloadPlanned()
    {
        planned().loadAll();
        m_app.drain();
        return planned().workouts();
    }

    ExerciseDefinitionModel* definition(const QString& name)
    {
        for (auto* model : catalog().exercises())
        {
            if (model->name() == name)
                return model;
        }
        return nullptr;
    }

    void addExercise(const QString& name)
    {
        ExerciseDefinitionModel* model = definition(name);
        EXPECT_NE(model, nullptr) << name.toStdString();
        editor().addExercise(model);
    }

    void startWorkout(const QString& name = QStringLiteral("Leg Day"))
    {
        editor().createNew(name, QDateTime(QDate(2025, 1, 3), QTime(18, 0, 0)));
    }

    TestApplication m_app;
};

// --- lifecycle ---

TEST_F(WorkoutEditorTest, NothingIsEditableBeforeAWorkoutIsStarted)
{
    EXPECT_FALSE(editor().isEditing());
    EXPECT_EQ(editor().workout(), nullptr);
    EXPECT_FALSE(editor().isValid());
    EXPECT_EQ(editor().exerciseCount(), 0);

    editor().addExercise(definition(QStringLiteral("Back Squat")));

    EXPECT_EQ(editor().exerciseCount(), 0);
}

TEST_F(WorkoutEditorTest, CreateNewStartsACleanPlannedWorkout)
{
    startWorkout();

    EXPECT_TRUE(editor().isEditing());
    EXPECT_FALSE(editor().isDirty());
    EXPECT_EQ(editor().name(), QStringLiteral("Leg Day"));
    EXPECT_EQ(editor().plannedTime(), QDateTime(QDate(2025, 1, 3), QTime(18, 0, 0)));
    ASSERT_NE(editor().workout(), nullptr);
    EXPECT_EQ(editor().workout()->status(), WorkoutStatus::Planned);
}

TEST_F(WorkoutEditorTest, DiscardLeavesTheEditorEmpty)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));

    editor().discard();

    EXPECT_FALSE(editor().isEditing());
    EXPECT_FALSE(editor().isDirty());
    EXPECT_EQ(editor().workout(), nullptr);
}

// --- exercises ---

TEST_F(WorkoutEditorTest, AddingAnExerciseLinksItToTheCatalogAndSeedsOneSet)
{
    startWorkout();

    addExercise(QStringLiteral("Back Squat"));

    ASSERT_EQ(editor().exerciseCount(), 1);
    EXPECT_TRUE(editor().isDirty());

    const ExerciseModel* exercise = editor().workout()->exercises().first();
    EXPECT_EQ(exercise->name(), QStringLiteral("Back Squat"));
    EXPECT_EQ(exercise->kindString(), QStringLiteral("strength"));
    EXPECT_EQ(exercise->restSeconds(), 180);
    ASSERT_EQ(exercise->sets().size(), 1);
    EXPECT_EQ(exercise->sets().first()->metric(), QStringLiteral("reps"));
    EXPECT_EQ(exercise->sets().first()->repetitions(), 8);
}

TEST_F(WorkoutEditorTest, TheSeededSetFollowsTheDefinitionMetric)
{
    startWorkout();

    addExercise(QStringLiteral("Plank"));
    addExercise(QStringLiteral("Row 2k"));

    const auto exercises = editor().workout()->exercises();
    ASSERT_EQ(exercises.size(), 2);
    EXPECT_EQ(exercises[0]->sets().first()->metric(), QStringLiteral("duration"));
    EXPECT_EQ(exercises[0]->sets().first()->durationSeconds(), 30);
    EXPECT_EQ(exercises[1]->sets().first()->metric(), QStringLiteral("distance"));
    EXPECT_DOUBLE_EQ(exercises[1]->sets().first()->distanceMeters(), 1000.0);
}

TEST_F(WorkoutEditorTest, ExercisesCanBeReorderedAndRemoved)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    addExercise(QStringLiteral("Plank"));
    addExercise(QStringLiteral("Row 2k"));

    editor().moveExercise(2, 0);

    auto names = QStringList();
    for (const auto* exercise : editor().workout()->exercises())
        names.append(exercise->name());
    EXPECT_EQ(names,
              QStringList({ QStringLiteral("Row 2k"), QStringLiteral("Back Squat"),
                            QStringLiteral("Plank") }));

    editor().removeExercise(1);

    EXPECT_EQ(editor().exerciseCount(), 2);
    EXPECT_EQ(editor().workout()->exercises().at(1)->name(), QStringLiteral("Plank"));
}

TEST_F(WorkoutEditorTest, OutOfRangeExerciseIndexesAreIgnored)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));

    editor().removeExercise(5);
    editor().removeExercise(-1);
    editor().moveExercise(0, 9);

    EXPECT_EQ(editor().exerciseCount(), 1);
}

// --- sets ---

TEST_F(WorkoutEditorTest, AddSetCopiesTheLastSetOfTheExercise)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    editor().setSetRepetitions(0, 0, 5);
    editor().setSetWeight(0, 0, 100.0);

    editor().addSet(0);

    const auto sets = editor().workout()->exercises().first()->sets();
    ASSERT_EQ(sets.size(), 2);
    EXPECT_EQ(sets[1]->repetitions(), 5);
    EXPECT_DOUBLE_EQ(sets[1]->weight(), 100.0);
    EXPECT_FALSE(sets[1]->completed());
}

TEST_F(WorkoutEditorTest, AddSetSeedsAFreshSetWhenTheExerciseHasNoneLeft)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    editor().removeSet(0, 0);
    ASSERT_TRUE(editor().workout()->exercises().first()->sets().isEmpty());

    editor().addSet(0);

    const auto sets = editor().workout()->exercises().first()->sets();
    ASSERT_EQ(sets.size(), 1);
    EXPECT_EQ(sets[0]->entity().metric(), SetMetric::Reps);
    EXPECT_EQ(sets[0]->repetitions(), 8);
}

TEST_F(WorkoutEditorTest, ASeededSetFollowsTheMetricOfTheExerciseKind)
{
    startWorkout();
    addExercise(QStringLiteral("Plank"));
    editor().removeSet(0, 0);

    editor().addSet(0);

    const auto sets = editor().workout()->exercises().first()->sets();
    ASSERT_EQ(sets.size(), 1);
    EXPECT_EQ(sets[0]->entity().metric(), SetMetric::Duration);
    EXPECT_EQ(sets[0]->durationSeconds(), 30);
}

TEST_F(WorkoutEditorTest, SetsCanBeEditedDuplicatedReorderedAndRemoved)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    editor().setSetRepetitions(0, 0, 5);
    editor().setSetWeight(0, 0, 100.0);
    editor().addSet(0);
    editor().setSetWeight(0, 1, 110.0);

    editor().duplicateSet(0, 0);
    ASSERT_EQ(editor().workout()->exercises().first()->sets().size(), 3);

    editor().moveSet(0, 2, 0);
    EXPECT_DOUBLE_EQ(editor().workout()->exercises().first()->sets().at(0)->weight(), 110.0);

    editor().removeSet(0, 0);
    const auto sets = editor().workout()->exercises().first()->sets();
    ASSERT_EQ(sets.size(), 2);
    EXPECT_DOUBLE_EQ(sets[0]->weight(), 100.0);
}

TEST_F(WorkoutEditorTest, DurationAndDistanceSetsAreEditableToo)
{
    startWorkout();
    addExercise(QStringLiteral("Plank"));
    addExercise(QStringLiteral("Row 2k"));

    editor().setSetDuration(0, 0, 90);
    editor().setSetDistance(1, 0, 2000.0);

    EXPECT_EQ(editor().workout()->exercises().at(0)->sets().first()->durationSeconds(), 90);
    EXPECT_DOUBLE_EQ(editor().workout()->exercises().at(1)->sets().first()->distanceMeters(),
                     2000.0);
}

TEST_F(WorkoutEditorTest, NegativeSetValuesAreRejected)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    editor().setSetRepetitions(0, 0, 5);

    editor().setSetRepetitions(0, 0, -3);

    EXPECT_EQ(editor().workout()->exercises().first()->sets().first()->repetitions(), 5);
}

// --- validation and persistence ---

TEST_F(WorkoutEditorTest, AWorkoutWithoutANameIsNotValid)
{
    editor().createNew(QStringLiteral("  "), QDateTime::currentDateTime());
    addExercise(QStringLiteral("Back Squat"));

    EXPECT_FALSE(editor().isValid());
    EXPECT_FALSE(editor().validationErrors().isEmpty());
}

TEST_F(WorkoutEditorTest, ValidationErrorsNameTheMissingNameAndClearOnceItIsFilledIn)
{
    editor().createNew(QStringLiteral(""), QDateTime::currentDateTime());
    addExercise(QStringLiteral("Back Squat"));

    ASSERT_FALSE(editor().isValid());
    const QStringList errors = editor().validationErrors();
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains(QStringLiteral("name"))) << errors.first().toStdString();

    editor().setName(QStringLiteral("Leg Day"));

    EXPECT_TRUE(editor().isValid());
    EXPECT_TRUE(editor().validationErrors().isEmpty());
}

TEST_F(WorkoutEditorTest, ClearingTheNameBlocksSavingAgainAndSaysWhy)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    ASSERT_TRUE(editor().isValid());

    editor().setName(QStringLiteral(""));

    EXPECT_FALSE(editor().isValid());
    ASSERT_EQ(editor().validationErrors().size(), 1);
    EXPECT_TRUE(editor().validationErrors().first().contains(QStringLiteral("name")));
}

TEST_F(WorkoutEditorTest, AClosedEditorOffersNoValidationHint)
{
    EXPECT_TRUE(editor().validationErrors().isEmpty());

    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    editor().discard();

    EXPECT_TRUE(editor().validationErrors().isEmpty());
}

TEST_F(WorkoutEditorTest, AReopenedWorkoutIsSaveableBeforeAnythingIsChanged)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));

    QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);
    editor().save();
    m_app.drain();
    ASSERT_EQ(saved.count(), 1);
    const int workoutId = saved.first().first().toInt();

    editor().discard();
    editor().edit(workoutId);
    m_app.drain();

    EXPECT_TRUE(editor().isValid());
    EXPECT_FALSE(editor().isDirty());
    EXPECT_TRUE(editor().validationErrors().isEmpty());

    editor().save();
    m_app.drain();

    ASSERT_EQ(saved.count(), 2);
    EXPECT_EQ(saved.at(1).first().toInt(), workoutId);
    EXPECT_EQ(reloadPlanned().size(), 1);
}

TEST_F(WorkoutEditorTest, SaveRefusesAnInvalidWorkoutAndReportsWhy)
{
    editor().createNew(QStringLiteral(""), QDateTime::currentDateTime());
    addExercise(QStringLiteral("Back Squat"));

    QSignalSpy errors(&editor(), &WorkoutEditorViewModel::errorOccurred);
    QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);

    editor().save();
    m_app.drain();

    EXPECT_EQ(saved.count(), 0);
    ASSERT_EQ(errors.count(), 1);
    EXPECT_TRUE(errors.first().first().toString().contains(QStringLiteral("name")));
}

TEST_F(WorkoutEditorTest, SaveStoresTheWorkoutAndClearsTheDirtyFlag)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    editor().setSetRepetitions(0, 0, 5);
    editor().setSetWeight(0, 0, 100.0);
    editor().addSet(0);

    QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);
    editor().save();
    m_app.drain();

    ASSERT_EQ(saved.count(), 1);
    EXPECT_FALSE(editor().isDirty());

    const int workoutId = saved.first().first().toInt();
    std::optional<Workout> reloaded;
    m_app.workoutService()
        .findWorkout(workoutId)
        .then(&editor(), [&reloaded](std::optional<Workout> found) { reloaded = found; })
        .warnOnError("reload the saved workout");
    m_app.drain();

    ASSERT_TRUE(reloaded.has_value());
    EXPECT_EQ(reloaded->name(), QStringLiteral("Leg Day"));
    ASSERT_EQ(reloaded->exercises().size(), 1u);
    EXPECT_EQ(reloaded->exercises()[0].name(), QStringLiteral("Back Squat"));
    EXPECT_TRUE(reloaded->exercises()[0].hasDefinition());
    EXPECT_EQ(reloaded->exercises()[0].sets().size(), 2u);
}

TEST_F(WorkoutEditorTest, EditReopensAStoredWorkout)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);
    editor().save();
    m_app.drain();
    ASSERT_EQ(saved.count(), 1);
    const int workoutId = saved.first().first().toInt();

    editor().discard();
    ASSERT_FALSE(editor().isEditing());

    editor().edit(workoutId);
    m_app.drain();

    EXPECT_TRUE(editor().isEditing());
    EXPECT_FALSE(editor().isDirty());
    EXPECT_EQ(editor().name(), QStringLiteral("Leg Day"));
    EXPECT_EQ(editor().exerciseCount(), 1);
}

TEST_F(WorkoutEditorTest, EditFillsTheEditorWithTheStoredNameExercisesAndSets)
{
    startWorkout(QStringLiteral("Push Day"));
    addExercise(QStringLiteral("Back Squat"));
    editor().setSetRepetitions(0, 0, 5);
    editor().setSetWeight(0, 0, 100.0);
    editor().addSet(0);
    addExercise(QStringLiteral("Plank"));

    QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);
    editor().save();
    m_app.drain();
    ASSERT_EQ(saved.count(), 1);
    const int workoutId = saved.first().first().toInt();

    editor().discard();
    editor().edit(workoutId);
    m_app.drain();

    EXPECT_EQ(editor().name(), QStringLiteral("Push Day"));
    ASSERT_NE(editor().workout(), nullptr);
    EXPECT_EQ(editor().workout()->id(), workoutId);

    const auto exercises = editor().workout()->exercises();
    ASSERT_EQ(exercises.size(), 2);
    EXPECT_EQ(exercises[0]->name(), QStringLiteral("Back Squat"));
    EXPECT_EQ(exercises[1]->name(), QStringLiteral("Plank"));
    ASSERT_EQ(exercises[0]->sets().size(), 2);
    EXPECT_EQ(exercises[0]->sets().first()->repetitions(), 5);
    EXPECT_DOUBLE_EQ(exercises[0]->sets().first()->weight(), 100.0);
}

TEST_F(WorkoutEditorTest, EditingAPlannedWorkoutUpdatesItInPlaceInsteadOfAddingASecondOne)
{
    startWorkout(QStringLiteral("Push Day"));
    addExercise(QStringLiteral("Back Squat"));

    QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);
    editor().save();
    m_app.drain();
    ASSERT_EQ(saved.count(), 1);
    const int workoutId = saved.first().first().toInt();

    auto before = reloadPlanned();
    ASSERT_EQ(before.size(), 1);
    ASSERT_EQ(before.first()->exercises().first()->sets().size(), 1);

    editor().discard();
    editor().edit(workoutId);
    m_app.drain();
    ASSERT_TRUE(editor().isEditing());

    editor().addSet(0);
    EXPECT_TRUE(editor().isDirty());
    editor().save();
    m_app.drain();

    ASSERT_EQ(saved.count(), 2);
    EXPECT_EQ(saved.at(1).first().toInt(), workoutId);

    auto after = reloadPlanned();
    ASSERT_EQ(after.size(), 1);
    EXPECT_EQ(after.first()->id(), workoutId);
    EXPECT_EQ(after.first()->name(), QStringLiteral("Push Day"));
    ASSERT_EQ(after.first()->exercises().size(), 1);
    EXPECT_EQ(after.first()->exercises().first()->sets().size(), 2);
}

TEST_F(WorkoutEditorTest, RenamingAnEditedWorkoutKeepsItInThePlannedList)
{
    startWorkout(QStringLiteral("Push Day"));
    addExercise(QStringLiteral("Back Squat"));

    QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);
    editor().save();
    m_app.drain();
    const int workoutId = saved.first().first().toInt();

    editor().discard();
    editor().edit(workoutId);
    m_app.drain();

    editor().setName(QStringLiteral("Push Day A"));
    editor().save();
    m_app.drain();

    auto after = reloadPlanned();
    ASSERT_EQ(after.size(), 1);
    EXPECT_EQ(after.first()->id(), workoutId);
    EXPECT_EQ(after.first()->name(), QStringLiteral("Push Day A"));
    EXPECT_EQ(after.first()->status(), WorkoutStatus::Planned);
}

TEST_F(WorkoutEditorTest, EditReportsAMissingWorkout)
{
    QSignalSpy errors(&editor(), &WorkoutEditorViewModel::errorOccurred);

    editor().edit(4242);
    m_app.drain();

    EXPECT_FALSE(editor().isEditing());
    EXPECT_EQ(errors.count(), 1);
}

// --- templates ---

TEST_F(WorkoutEditorTest, SaveAsTemplateStoresAReusableTemplate)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    editor().setSetRepetitions(0, 0, 5);
    editor().setSetWeight(0, 0, 100.0);

    QSignalSpy templated(&editor(), &WorkoutEditorViewModel::savedAsTemplate);
    editor().saveAsTemplate(QStringLiteral("Squat Day"));
    m_app.drain();

    ASSERT_EQ(templated.count(), 1);
    EXPECT_GT(templated.first().first().toInt(), 0);
}

TEST_F(WorkoutEditorTest, StartFromTemplateFillsTheEditorAndMarksItDirty)
{
    startWorkout();
    addExercise(QStringLiteral("Back Squat"));
    editor().setSetRepetitions(0, 0, 5);
    editor().setSetWeight(0, 0, 100.0);
    editor().addSet(0);

    QSignalSpy templated(&editor(), &WorkoutEditorViewModel::savedAsTemplate);
    editor().saveAsTemplate(QStringLiteral("Squat Day"));
    m_app.drain();
    ASSERT_EQ(templated.count(), 1);
    const int templateId = templated.first().first().toInt();

    editor().discard();
    editor().startFromTemplate(templateId, QDateTime(QDate(2025, 2, 1), QTime(9, 0, 0)));
    m_app.drain();

    EXPECT_TRUE(editor().isEditing());
    EXPECT_TRUE(editor().isDirty());
    EXPECT_EQ(editor().name(), QStringLiteral("Squat Day"));
    EXPECT_EQ(editor().plannedTime(), QDateTime(QDate(2025, 2, 1), QTime(9, 0, 0)));
    ASSERT_EQ(editor().exerciseCount(), 1);

    const ExerciseModel* exercise = editor().workout()->exercises().first();
    EXPECT_EQ(exercise->name(), QStringLiteral("Back Squat"));
    ASSERT_EQ(exercise->sets().size(), 2);
    EXPECT_EQ(exercise->sets().first()->repetitions(), 5);
    EXPECT_DOUBLE_EQ(exercise->sets().first()->weight(), 100.0);
}

TEST_F(WorkoutEditorTest, AWorkoutBuiltFromScratchNeedsNoAiImport)
{
    startWorkout(QStringLiteral("Full Body"));
    addExercise(QStringLiteral("Back Squat"));
    editor().setSetRepetitions(0, 0, 5);
    editor().setSetWeight(0, 0, 100.0);
    editor().addSet(0);
    editor().addSet(0);
    addExercise(QStringLiteral("Plank"));
    editor().setSetDuration(1, 0, 60);

    ASSERT_TRUE(editor().isValid());

    QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);
    editor().save();
    m_app.drain();

    ASSERT_EQ(saved.count(), 1);

    std::optional<Workout> reloaded;
    m_app.workoutService()
        .findWorkout(saved.first().first().toInt())
        .then(&editor(), [&reloaded](std::optional<Workout> found) { reloaded = found; })
        .warnOnError("reload the workout built from scratch");
    m_app.drain();

    ASSERT_TRUE(reloaded.has_value());
    EXPECT_EQ(reloaded->totalSets(), 4);
    EXPECT_EQ(reloaded->exercises().size(), 2u);
}
