#include <QSignalSpy>
#include <gtest/gtest.h>

#include "domain/exercisecatalog/exercisedefinition.h"
#include "testapplication.h"
#include "ui/models/exercisedefinitionmodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouteditorviewmodel.h"

class PlannedDayTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_app.setCurrentDateTime(QDateTime(QDate(2026, 9, 10), QTime(9, 0, 0)));

        ExerciseDefinition definition(QStringLiteral("back-squat"), QStringLiteral("Back Squat"),
                                      ExerciseKind::Strength);
        definition.setDefaultMetric(SetMetric::Reps);
        definition.setDefaultLoadType(LoadType::External);
        definition.setDefaultRestSeconds(180);
        definition.addMuscle({ Muscle::Quads, MuscleRole::Primary });
        m_app.seedDefinition(definition);
        m_app.drain();

        catalog().load();
        m_app.drain();
    }

    ExerciseCatalogViewModel& catalog() { return m_app.exerciseCatalogViewModel(); }
    WorkoutEditorViewModel& editor() { return m_app.workoutEditorViewModel(); }
    PlannedWorkoutViewModel& planned() { return m_app.plannedWorkoutViewModel(); }

    void composeWorkout(const QString& name)
    {
        editor().createNew(name, QDateTime(QDate(2026, 9, 10), QTime(9, 0, 0)));
        editor().addExercise(catalog().exercises().first());
        m_app.drain();
    }

    void save()
    {
        QSignalSpy saved(&editor(), &WorkoutEditorViewModel::saved);
        editor().save();
        m_app.drain();
        EXPECT_EQ(saved.count(), 1);
    }

    QList<WorkoutModel*> reloadPlanned()
    {
        planned().loadAll();
        m_app.drain();
        return planned().workouts();
    }

    static QDate dayOf(const WorkoutModel* workout) { return workout->plannedTime().date(); }

    TestApplication m_app;
};

TEST_F(PlannedDayTest, TheEditorStartsOnToday)
{
    composeWorkout(QStringLiteral("Leg Day"));

    EXPECT_EQ(editor().plannedTime().date(), QDate(2026, 9, 10));
    EXPECT_EQ(editor().plannedDayText(), QStringLiteral("Today · Thu, 10 Sep 2026"));
}

TEST_F(PlannedDayTest, TheDayCanBeMovedForwardInTheEditor)
{
    composeWorkout(QStringLiteral("Leg Day"));

    editor().shiftPlannedDay(1);
    editor().shiftPlannedDay(1);

    EXPECT_EQ(editor().plannedTime().date(), QDate(2026, 9, 12));
    EXPECT_EQ(editor().plannedDayText(), QStringLiteral("Sat, 12 Sep 2026"));
}

TEST_F(PlannedDayTest, TheDayCanBeMovedBackAgain)
{
    composeWorkout(QStringLiteral("Leg Day"));

    editor().shiftPlannedDay(3);
    editor().shiftPlannedDay(-2);

    EXPECT_EQ(editor().plannedTime().date(), QDate(2026, 9, 11));
    EXPECT_EQ(editor().plannedDayText(), QStringLiteral("Tomorrow · Fri, 11 Sep 2026"));
}

TEST_F(PlannedDayTest, TheChosenDayIsWhatTheListShows)
{
    composeWorkout(QStringLiteral("Leg Day"));
    editor().shiftPlannedDay(4);
    save();

    const QList<WorkoutModel*> workouts = reloadPlanned();
    ASSERT_EQ(workouts.size(), 1);
    EXPECT_EQ(dayOf(workouts.first()), QDate(2026, 9, 14));
    EXPECT_NE(dayOf(workouts.first()), QDate(2026, 9, 10));
}

TEST_F(PlannedDayTest, TheChosenDaySurvivesAReload)
{
    composeWorkout(QStringLiteral("Leg Day"));
    editor().shiftPlannedDay(2);
    save();

    reloadPlanned();
    const QList<WorkoutModel*> workouts = reloadPlanned();

    ASSERT_EQ(workouts.size(), 1);
    EXPECT_EQ(dayOf(workouts.first()), QDate(2026, 9, 12));
}

TEST_F(PlannedDayTest, TheListStandsInDayOrderNotInTheOrderTheyWereMade)
{
    composeWorkout(QStringLiteral("Friday session"));
    editor().shiftPlannedDay(5);
    save();

    composeWorkout(QStringLiteral("Monday session"));
    editor().shiftPlannedDay(1);
    save();

    const QList<WorkoutModel*> workouts = reloadPlanned();
    ASSERT_EQ(workouts.size(), 2);
    EXPECT_EQ(workouts[0]->name(), QStringLiteral("Monday session"));
    EXPECT_EQ(workouts[1]->name(), QStringLiteral("Friday session"));
    EXPECT_LT(dayOf(workouts[0]), dayOf(workouts[1]));
}

TEST_F(PlannedDayTest, TheNextWorkoutIsTheNearestDay)
{
    composeWorkout(QStringLiteral("Later"));
    editor().shiftPlannedDay(6);
    save();

    composeWorkout(QStringLiteral("Sooner"));
    editor().shiftPlannedDay(2);
    save();

    reloadPlanned();

    ASSERT_NE(planned().nextWorkout(), nullptr);
    EXPECT_EQ(planned().nextWorkout()->name(), QStringLiteral("Sooner"));
}

TEST_F(PlannedDayTest, AnImportedPlanKeepsItsOwnDates)
{
    const QString plan
        = QStringLiteral("{\"user_profile\": null, \"workouts\": ["
                         "{\"name\": \"AI Monday\", \"planned_time\": \"2026-09-14T18:00:00\","
                         " \"exercises\": [{\"name\": \"Back Squat\", \"sets\": \"5x40kg\"}]},"
                         "{\"name\": \"AI Wednesday\", \"planned_time\": \"2026-09-16T18:00:00\","
                         " \"exercises\": [{\"name\": \"Back Squat\", \"sets\": \"5x40kg\"}]}]}");

    planned().importFromJson(plan);
    const QList<WorkoutModel*> workouts = reloadPlanned();

    ASSERT_EQ(workouts.size(), 2);
    EXPECT_EQ(workouts[0]->name(), QStringLiteral("AI Monday"));
    EXPECT_EQ(dayOf(workouts[0]), QDate(2026, 9, 14));
    EXPECT_EQ(dayOf(workouts[1]), QDate(2026, 9, 16));
    EXPECT_NE(dayOf(workouts[0]), QDate(2026, 9, 10));
}

TEST_F(PlannedDayTest, AnImportedPlanWithoutDatesStillGetsSpreadOverTheDaysAhead)
{
    const QString plan
        = QStringLiteral("{\"user_profile\": null, \"workouts\": ["
                         "{\"name\": \"Day one\", \"exercises\": [{\"name\": \"Back Squat\","
                         " \"sets\": \"5x40kg\"}]}]}");

    planned().importFromJson(plan);
    const QList<WorkoutModel*> workouts = reloadPlanned();

    ASSERT_EQ(workouts.size(), 1);
    EXPECT_EQ(dayOf(workouts.first()), QDate(2026, 9, 11));
}

TEST_F(PlannedDayTest, ADayInThePastIsAllowedAndStaysOnTheList)
{
    composeWorkout(QStringLiteral("Missed session"));
    editor().shiftPlannedDay(-1);
    save();

    const QList<WorkoutModel*> workouts = reloadPlanned();
    ASSERT_EQ(workouts.size(), 1);
    EXPECT_EQ(dayOf(workouts.first()), QDate(2026, 9, 9));
    EXPECT_EQ(workouts.first()->status(), WorkoutStatus::Planned);
}

TEST_F(PlannedDayTest, ShiftingWithoutAnOpenEditorChangesNothing)
{
    editor().discard();
    editor().shiftPlannedDay(1);

    EXPECT_EQ(editor().plannedDayText(), QString());
}
