#include "domain/exercisecatalog/exercisedefinition.h"
#include "domain/workout/setprescription.h"
#include "domain/workout/templateexercise.h"
#include "domain/workout/workouttemplate.h"
#include "testapplication.h"
#include "ui/models/exercisedefinitionmodel.h"
#include "ui/models/exercisemodel.h"
#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/models/workouttemplatemodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"
#include "ui/viewmodels/workouteditorviewmodel.h"
#include "ui/viewmodels/workouttemplateviewmodel.h"

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

class WorkoutTemplateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_app.seedDefinition(makeDefinition(QStringLiteral("back-squat"),
                                            QStringLiteral("Back Squat"), ExerciseKind::Strength,
                                            SetMetric::Reps, LoadType::External, 180));
        m_app.seedDefinition(makeDefinition(QStringLiteral("bench-press"),
                                            QStringLiteral("Bench Press"), ExerciseKind::Strength,
                                            SetMetric::Reps, LoadType::External, 150));
        m_app.drain();

        catalog().load();
        m_app.drain();
    }

    ExerciseCatalogViewModel& catalog() { return m_app.exerciseCatalogViewModel(); }
    WorkoutEditorViewModel& editor() { return m_app.workoutEditorViewModel(); }
    WorkoutTemplateViewModel& templates() { return m_app.workoutTemplateViewModel(); }

    ExerciseDefinitionModel* definition(const QString& name)
    {
        for (auto* model : catalog().exercises())
        {
            if (model->name() == name)
                return model;
        }
        return nullptr;
    }

    int saveTemplateFromEditor(const QString& templateName, const QStringList& exerciseNames)
    {
        editor().createNew(templateName, QDateTime(QDate(2025, 1, 3), QTime(18, 0, 0)));
        for (const QString& name : exerciseNames)
            editor().addExercise(definition(name));

        QSignalSpy templated(&editor(), &WorkoutEditorViewModel::savedAsTemplate);
        editor().saveAsTemplate(templateName);
        m_app.drain();
        EXPECT_EQ(templated.count(), 1) << templateName.toStdString();
        editor().discard();

        return templated.isEmpty() ? -1 : templated.first().first().toInt();
    }

    WorkoutTemplateModel* templateNamed(const QString& name)
    {
        for (auto* model : templates().templates())
        {
            if (model->name() == name)
                return model;
        }
        return nullptr;
    }

    TestApplication m_app;
};

TEST_F(WorkoutTemplateTest, TheListStartsEmpty)
{
    templates().load();
    m_app.drain();

    EXPECT_EQ(templates().count(), 0);
    EXPECT_TRUE(templates().isEmpty());
}

TEST_F(WorkoutTemplateTest, ATemplateSavedFromTheEditorShowsUpInTheList)
{
    saveTemplateFromEditor(QStringLiteral("Squat Day"), { QStringLiteral("Back Squat") });

    templates().load();
    m_app.drain();

    ASSERT_EQ(templates().count(), 1);
    EXPECT_EQ(templates().templates().first()->name(), QStringLiteral("Squat Day"));
}

TEST_F(WorkoutTemplateTest, TheListPreviewsExerciseNamesAndCounts)
{
    editor().createNew(QStringLiteral("Push Day"), QDateTime(QDate(2025, 1, 3), QTime(18, 0, 0)));
    editor().addExercise(definition(QStringLiteral("Back Squat")));
    editor().addExercise(definition(QStringLiteral("Bench Press")));
    editor().addSet(1);

    QSignalSpy templated(&editor(), &WorkoutEditorViewModel::savedAsTemplate);
    editor().saveAsTemplate(QStringLiteral("Push Day"));
    m_app.drain();
    ASSERT_EQ(templated.count(), 1);

    templates().load();
    m_app.drain();

    ASSERT_EQ(templates().count(), 1);
    const WorkoutTemplateModel* model = templates().templates().first();
    EXPECT_EQ(model->exerciseCount(), 2);
    EXPECT_EQ(model->setCount(), 3);
    EXPECT_EQ(model->exerciseNames(),
              QStringList({ QStringLiteral("Back Squat"), QStringLiteral("Bench Press") }));
    EXPECT_TRUE(model->isComplete());
}

TEST_F(WorkoutTemplateTest, SearchNarrowsTheListByName)
{
    saveTemplateFromEditor(QStringLiteral("Squat Day"), { QStringLiteral("Back Squat") });
    saveTemplateFromEditor(QStringLiteral("Bench Day"), { QStringLiteral("Bench Press") });

    templates().setSearchText(QStringLiteral("squat"));
    m_app.drain();

    ASSERT_EQ(templates().count(), 1);
    EXPECT_EQ(templates().templates().first()->name(), QStringLiteral("Squat Day"));

    templates().setSearchText(QString());
    m_app.drain();

    EXPECT_EQ(templates().count(), 2);
}

TEST_F(WorkoutTemplateTest, DuplicatingATemplateKeepsBothAndRefreshesTheList)
{
    const int id
        = saveTemplateFromEditor(QStringLiteral("Squat Day"), { QStringLiteral("Back Squat") });
    templates().load();
    m_app.drain();

    QSignalSpy duplicated(&templates(), &WorkoutTemplateViewModel::duplicated);
    templates().duplicate(id);
    m_app.drain();

    EXPECT_EQ(duplicated.count(), 1);
    ASSERT_EQ(templates().count(), 2);
    EXPECT_NE(templateNamed(QStringLiteral("Squat Day")), nullptr);
    EXPECT_NE(templateNamed(QStringLiteral("Squat Day (copy)")), nullptr);
}

TEST_F(WorkoutTemplateTest, RemovingATemplateDropsItFromTheList)
{
    const int id
        = saveTemplateFromEditor(QStringLiteral("Squat Day"), { QStringLiteral("Back Squat") });
    saveTemplateFromEditor(QStringLiteral("Bench Day"), { QStringLiteral("Bench Press") });
    templates().load();
    m_app.drain();
    ASSERT_EQ(templates().count(), 2);

    QSignalSpy removed(&templates(), &WorkoutTemplateViewModel::removed);
    templates().remove(id);
    m_app.drain();

    EXPECT_EQ(removed.count(), 1);
    ASSERT_EQ(templates().count(), 1);
    EXPECT_EQ(templates().templates().first()->name(), QStringLiteral("Bench Day"));
}

TEST_F(WorkoutTemplateTest, AnUnknownTemplateIsNeitherDuplicatedNorRemoved)
{
    saveTemplateFromEditor(QStringLiteral("Squat Day"), { QStringLiteral("Back Squat") });
    templates().load();
    m_app.drain();

    templates().duplicate(4242);
    templates().remove(4242);
    m_app.drain();

    EXPECT_EQ(templates().count(), 1);
}

TEST_F(WorkoutTemplateTest, ATemplateRoundTripsBackIntoTheEditor)
{
    editor().createNew(QStringLiteral("Squat Day"), QDateTime(QDate(2025, 1, 3), QTime(18, 0, 0)));
    editor().addExercise(definition(QStringLiteral("Back Squat")));
    editor().setSetRepetitions(0, 0, 5);
    editor().setSetWeight(0, 0, 100.0);

    QSignalSpy templated(&editor(), &WorkoutEditorViewModel::savedAsTemplate);
    editor().saveAsTemplate(QStringLiteral("Squat Day"));
    m_app.drain();
    ASSERT_EQ(templated.count(), 1);
    editor().discard();

    templates().load();
    m_app.drain();
    ASSERT_EQ(templates().count(), 1);

    editor().startFromTemplate(templates().templates().first()->templateId(),
                               QDateTime(QDate(2025, 2, 1), QTime(9, 0, 0)));
    m_app.drain();

    ASSERT_EQ(editor().exerciseCount(), 1);
    const ExerciseModel* exercise = editor().workout()->exercises().first();
    EXPECT_EQ(exercise->name(), QStringLiteral("Back Squat"));
    ASSERT_EQ(exercise->sets().size(), 1);
    EXPECT_EQ(exercise->sets().first()->repetitions(), 5);
    EXPECT_DOUBLE_EQ(exercise->sets().first()->weight(), 100.0);
}

TEST_F(WorkoutTemplateTest, ATemplatePointingAtAMissingExerciseIsFlaggedIncomplete)
{
    WorkoutTemplate orphan(QStringLiteral("Ghost Day"));
    TemplateExercise exercise(4242);
    exercise.addSet(SetPrescription(8, 60.0));
    orphan.addExercise(exercise);
    m_app.seedTemplate(orphan);

    templates().load();
    m_app.drain();

    ASSERT_EQ(templates().count(), 1);
    const WorkoutTemplateModel* model = templates().templates().first();
    EXPECT_EQ(model->exerciseCount(), 1);
    EXPECT_TRUE(model->exerciseNames().isEmpty());
    EXPECT_FALSE(model->isComplete());
}
