#include "infrastructure/workout/workoutjson.h"
#include "domain/workout/exercise.h"
#include "domain/workout/set.h"
#include <QJsonDocument>
#include <QStringList>
#include <gtest/gtest.h>

namespace
{

QString formatSets(const std::vector<Set>& sets)
{
    Exercise e("Formatter", 90);
    for (const auto& s : sets)
        e.addSet(s);
    return e.setsToString();
}

}  // namespace

class SetsFormatTest : public ::testing::TestWithParam<QString>
{
};

TEST_P(SetsFormatTest, Roundtrip_ParseThenFormat_IsIdentity)
{
    const QString input = GetParam();
    QStringList errors;

    const std::vector<Set> sets = WorkoutJson::parseSets(input, &errors);

    EXPECT_TRUE(errors.isEmpty()) << errors.join("; ").toStdString();
    EXPECT_FALSE(sets.empty());
    EXPECT_EQ(formatSets(sets), input);
}

INSTANTIATE_TEST_SUITE_P(
    Grammar, SetsFormatTest,
    ::testing::Values(QStringLiteral("10x80kg"), QStringLiteral("10x80kg, 8x85kg, 6x90kg"),
                      QStringLiteral("10x82.5kg"), QStringLiteral("12xBW"),
                      QStringLiteral("8xBW+10kg"), QStringLiteral("8xBW-20kg"),
                      QStringLiteral("15xBAND"), QStringLiteral("45s"), QStringLiteral("3min"),
                      QStringLiteral("90s"), QStringLiteral("400m"), QStringLiteral("5km"),
                      QStringLiteral("5km@24min"), QStringLiteral("8x(20s/10s)"),
                      QStringLiteral("4x(400m/90s)"), QStringLiteral("5x(10x80kg/2min)"),
                      QStringLiteral("10x60kg, 8x(20s/10s), 5km@24min")));

class ParseSetsTest : public ::testing::Test
{
};

TEST_F(ParseSetsTest, LegacyWeightedFormat_IsUnchanged)
{
    QStringList errors;
    const std::vector<Set> sets = WorkoutJson::parseSets("5x60kg,5x75kg,5x85kg", &errors);

    ASSERT_EQ(sets.size(), 3u);
    EXPECT_TRUE(errors.isEmpty());
    EXPECT_EQ(sets[0].repetitions(), 5);
    EXPECT_DOUBLE_EQ(sets[0].weight(), 60.0);
    EXPECT_TRUE(sets[0].isWeighted());
    EXPECT_EQ(sets[0].restSecondsOverride(), -1);
    EXPECT_DOUBLE_EQ(sets[2].weight(), 85.0);
}

TEST_F(ParseSetsTest, Tabata_ExpandsIntoEightTimedSets)
{
    QStringList errors;
    const std::vector<Set> sets = WorkoutJson::parseSets("8x(20s/10s)", &errors);

    ASSERT_EQ(sets.size(), 8u);
    EXPECT_TRUE(errors.isEmpty());
    for (const auto& s : sets)
    {
        EXPECT_EQ(s.metric(), SetMetric::Duration);
        EXPECT_EQ(s.durationSeconds(), 20);
        EXPECT_EQ(s.restSecondsOverride(), 10);
        EXPECT_FALSE(s.isWeighted());
    }
}

TEST_F(ParseSetsTest, BodyweightVariants_MapToLoadTypes)
{
    QStringList errors;
    const std::vector<Set> sets
        = WorkoutJson::parseSets("12xBW, 8xBW+10kg, 8xBW-20kg, 15xBAND", &errors);

    ASSERT_EQ(sets.size(), 4u);
    EXPECT_TRUE(errors.isEmpty());
    EXPECT_EQ(sets[0].loadType(), LoadType::Bodyweight);
    EXPECT_EQ(sets[1].loadType(), LoadType::Added);
    EXPECT_DOUBLE_EQ(sets[1].weight(), 10.0);
    EXPECT_EQ(sets[2].loadType(), LoadType::Assisted);
    EXPECT_DOUBLE_EQ(sets[2].weight(), 20.0);
    EXPECT_EQ(sets[3].loadType(), LoadType::Band);
    for (const auto& s : sets)
        EXPECT_FALSE(s.isWeighted());
}

TEST_F(ParseSetsTest, DistanceWithTime_SplitsOnAtSign)
{
    QStringList errors;
    const std::vector<Set> sets = WorkoutJson::parseSets("5km@24min, 400m", &errors);

    ASSERT_EQ(sets.size(), 2u);
    EXPECT_TRUE(errors.isEmpty());
    EXPECT_EQ(sets[0].metric(), SetMetric::Distance);
    EXPECT_DOUBLE_EQ(sets[0].distanceMeters(), 5000.0);
    EXPECT_EQ(sets[0].durationSeconds(), 1440);
    EXPECT_DOUBLE_EQ(sets[1].distanceMeters(), 400.0);
    EXPECT_EQ(sets[1].durationSeconds(), 0);
}

TEST_F(ParseSetsTest, RestSeparator_WorksWithoutRepeatPrefix)
{
    QStringList errors;
    const std::vector<Set> sets = WorkoutJson::parseSets("20s/10s", &errors);

    ASSERT_EQ(sets.size(), 1u);
    EXPECT_TRUE(errors.isEmpty());
    EXPECT_EQ(sets[0].durationSeconds(), 20);
    EXPECT_EQ(sets[0].restSecondsOverride(), 10);
}

TEST_F(ParseSetsTest, ToleratesSpacingCaseAndUnitAliases)
{
    QStringList errors;
    const std::vector<Set> sets
        = WorkoutJson::parseSets(" 10 X 80 KG , 5 km , 45 sec , 12 x bw ", &errors);

    ASSERT_EQ(sets.size(), 4u);
    EXPECT_TRUE(errors.isEmpty()) << errors.join("; ").toStdString();
    EXPECT_DOUBLE_EQ(sets[0].weight(), 80.0);
    EXPECT_DOUBLE_EQ(sets[1].distanceMeters(), 5000.0);
    EXPECT_EQ(sets[2].durationSeconds(), 45);
    EXPECT_EQ(sets[3].loadType(), LoadType::Bodyweight);
}

TEST_F(ParseSetsTest, UnknownToken_IsReportedAndSkipped)
{
    QStringList errors;
    const std::vector<Set> sets = WorkoutJson::parseSets("10x80kg, gibberish, 8x85kg", &errors);

    EXPECT_EQ(sets.size(), 2u);
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains("gibberish"));
}

TEST_F(ParseSetsTest, MalformedRest_IsReportedNotSilentlyDropped)
{
    QStringList errors;
    const std::vector<Set> sets = WorkoutJson::parseSets("8x(20s/abc)", &errors);

    EXPECT_TRUE(sets.empty());
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains("20s/abc"));
}

TEST_F(ParseSetsTest, AbsurdRepeatCount_IsRejected)
{
    QStringList errors;
    const std::vector<Set> sets = WorkoutJson::parseSets("9999x(20s/10s)", &errors);

    EXPECT_TRUE(sets.empty());
    EXPECT_EQ(errors.size(), 1);
}

TEST_F(ParseSetsTest, WithoutErrorSink_StillParsesValidTokens)
{
    const std::vector<Set> sets = WorkoutJson::parseSets("10x80kg, gibberish");

    EXPECT_EQ(sets.size(), 1u);
}

class WorkoutJsonTest : public ::testing::Test
{
};

TEST_F(WorkoutJsonTest, FullJson_RoundtripsTimedSet)
{
    Set original = Set::createDuration(20);
    original.setRestSecondsOverride(10);
    original.setCompleted(true);

    const Set restored = WorkoutJson::setFromJson(WorkoutJson::setToJson(original));

    EXPECT_EQ(restored.metric(), SetMetric::Duration);
    EXPECT_EQ(restored.loadType(), LoadType::None);
    EXPECT_EQ(restored.durationSeconds(), 20);
    EXPECT_EQ(restored.restSecondsOverride(), 10);
    EXPECT_TRUE(restored.completed());
}

TEST_F(WorkoutJsonTest, FullJson_CachedSessionRemembersThatItNamesItself)
{
    Workout session(QStringLiteral("Freestyle · 18:30"),
                    QDateTime(QDate(2026, 9, 10), QTime(18, 30)));
    session.setGeneratedName(true);

    const Workout restored = WorkoutJson::workoutFromJson(WorkoutJson::workoutToJson(session));

    EXPECT_TRUE(restored.hasGeneratedName());
    EXPECT_EQ(restored.name(), QStringLiteral("Freestyle · 18:30"));
}

TEST_F(WorkoutJsonTest, FullJson_WorkoutNamedByTheLifterStaysThatWay)
{
    const Workout push(QStringLiteral("Push A"), QDateTime(QDate(2026, 9, 10), QTime(18, 30)));

    const Workout restored = WorkoutJson::workoutFromJson(WorkoutJson::workoutToJson(push));

    EXPECT_FALSE(restored.hasGeneratedName());
}

TEST_F(WorkoutJsonTest, FullJson_RoundtripsDistanceSet)
{
    const Set original = Set::createDistance(5000.0, 1440);

    const Set restored = WorkoutJson::setFromJson(WorkoutJson::setToJson(original));

    EXPECT_EQ(restored.metric(), SetMetric::Distance);
    EXPECT_DOUBLE_EQ(restored.distanceMeters(), 5000.0);
    EXPECT_EQ(restored.durationSeconds(), 1440);
}

TEST_F(WorkoutJsonTest, FullJson_LegacySetWithoutNewKeys_ReadsAsWeightedReps)
{
    QJsonObject legacy;
    legacy["repetitions"] = 10;
    legacy["weight"] = 80.0;
    legacy["completed"] = true;

    const Set s = WorkoutJson::setFromJson(legacy);

    EXPECT_EQ(s.metric(), SetMetric::Reps);
    EXPECT_EQ(s.loadType(), LoadType::External);
    EXPECT_EQ(s.restSecondsOverride(), -1);
    EXPECT_TRUE(s.isWeighted());
}

TEST_F(WorkoutJsonTest, FullJson_RoundtripsExerciseKind)
{
    Exercise original("Burpees", 60);
    original.setKind(ExerciseKind::Interval);
    Set work = Set::createDuration(20);
    work.setRestSecondsOverride(10);
    original.addSet(work);

    const Exercise restored = WorkoutJson::exerciseFromJson(WorkoutJson::exerciseToJson(original));

    EXPECT_EQ(restored.kind(), ExerciseKind::Interval);
    ASSERT_EQ(restored.sets().size(), 1u);
    EXPECT_EQ(restored.sets()[0].durationSeconds(), 20);
    EXPECT_EQ(restored.restSecondsForSet(0), 10);
}

TEST_F(WorkoutJsonTest, CompactJson_UsesGrammarAndCarriesKind)
{
    Exercise e("Burpees", 60);
    e.setKind(ExerciseKind::Interval);
    for (int i = 0; i < 8; ++i)
    {
        Set work = Set::createDuration(20);
        work.setRestSecondsOverride(10);
        e.addSet(work);
    }

    const QJsonObject compact = WorkoutJson::exerciseToJsonCompact(e);

    EXPECT_EQ(compact["sets"].toString(), "8x(20s/10s)");
    EXPECT_EQ(compact["kind"].toString(), "interval");
}

TEST_F(WorkoutJsonTest, CompactJson_RoundtripsThroughExerciseFromJson)
{
    Exercise original("Mixed", 90);
    original.setKind(ExerciseKind::Cardio);
    original.addSet(Set(10, 80.0));
    original.addSet(Set::createDistance(5000.0, 1440));

    QStringList errors;
    const Exercise restored
        = WorkoutJson::exerciseFromJson(WorkoutJson::exerciseToJsonCompact(original), &errors);

    EXPECT_TRUE(errors.isEmpty());
    EXPECT_EQ(restored.kind(), ExerciseKind::Cardio);
    ASSERT_EQ(restored.sets().size(), 2u);
    EXPECT_TRUE(restored.sets()[0].isWeighted());
    EXPECT_EQ(restored.sets()[1].metric(), SetMetric::Distance);
    EXPECT_EQ(restored.setsToString(), original.setsToString());
}

TEST_F(WorkoutJsonTest, CompactJson_MarksTheTickedSets)
{
    Exercise exercise("Bench Press", 120);
    Set done(5, 40.0);
    done.setCompleted(true);
    exercise.addSet(done);
    exercise.addSet(Set(5, 40.0));

    const QJsonObject compact = WorkoutJson::exerciseToJsonCompact(exercise);

    EXPECT_EQ(compact["sets"].toString(), "5x40kg!, 5x40kg");
}

TEST_F(WorkoutJsonTest, CompactJson_RoundtripsWhichSetsWereDone)
{
    Exercise original("Bench Press", 120);
    Set done(5, 40.0);
    done.setCompleted(true);
    original.addSet(done);
    original.addSet(Set(5, 40.0));

    QStringList errors;
    const Exercise restored
        = WorkoutJson::exerciseFromJson(WorkoutJson::exerciseToJsonCompact(original), &errors);

    EXPECT_TRUE(errors.isEmpty());
    ASSERT_EQ(restored.sets().size(), 2u);
    EXPECT_TRUE(restored.sets()[0].completed());
    EXPECT_FALSE(restored.sets()[1].completed());
}

TEST_F(WorkoutJsonTest, CompactJson_WithoutAnyMarkerReadsAsUntickedJustLikeBefore)
{
    QStringList errors;
    const Exercise restored = WorkoutJson::exerciseFromJson(
        QJsonDocument::fromJson(R"({"name": "Squat", "sets": "5x40kg, 5x40kg"})").object(),
        &errors);

    EXPECT_TRUE(errors.isEmpty());
    ASSERT_EQ(restored.sets().size(), 2u);
    EXPECT_FALSE(restored.sets()[0].completed());
    EXPECT_FALSE(restored.sets()[1].completed());
}

TEST_F(WorkoutJsonTest, CompactJson_AMarkerOnItsOwnIsReportedNotImported)
{
    QStringList errors;
    const std::vector<Set> sets = WorkoutJson::parseSets(QStringLiteral("!, 5x40kg"), &errors);

    ASSERT_EQ(sets.size(), 1u);
    EXPECT_FALSE(sets[0].completed());
    ASSERT_EQ(errors.size(), 1);
}

TEST_F(WorkoutJsonTest, CompactJson_AMarkedRepeatedRunTicksEverySetInIt)
{
    const std::vector<Set> sets = WorkoutJson::parseSets(QStringLiteral("3x(20s/10s)!"), nullptr);

    ASSERT_EQ(sets.size(), 3u);
    for (const Set& set : sets)
        EXPECT_TRUE(set.completed());
}

TEST_F(WorkoutJsonTest, WorkoutsFromJsonArray_CollectsErrorsFromEveryExercise)
{
    const QString json = R"([
        {"name": "Day 1", "exercises": [
            {"name": "Bench", "sets": "10x80kg"},
            {"name": "Broken", "sets": "nonsense"}
        ]}
    ])";

    QStringList errors;
    const auto workouts = WorkoutJson::workoutsFromJsonArray(
        QJsonDocument::fromJson(json.toUtf8()).array(), &errors);

    ASSERT_EQ(workouts.size(), 1u);
    ASSERT_EQ(workouts[0].exercises().size(), 2u);
    EXPECT_EQ(workouts[0].exercises()[0].sets().size(), 1u);
    EXPECT_TRUE(workouts[0].exercises()[1].sets().empty());
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains("nonsense"));
}

TEST(WorkoutJsonCatalogLinkTest, ADefinitionLinkSurvivesTheJsonRoundTrip)
{
    Exercise exercise
        = Exercise::createFromDefinition(42, "Bench Press", ExerciseKind::Strength, 180);
    exercise.setNotes("Paused reps.");
    exercise.addSet(Set(5, 100.0));

    const Exercise restored
        = WorkoutJson::exerciseFromJson(WorkoutJson::exerciseToJson(exercise), nullptr);

    ASSERT_TRUE(restored.hasDefinition());
    EXPECT_EQ(restored.definitionId().value(), 42);
    EXPECT_EQ(restored.notes(), "Paused reps.");
}

TEST(WorkoutJsonCatalogLinkTest, AnAdHocExerciseCarriesNoDefinitionKey)
{
    Exercise exercise = Exercise::createAdHoc("Zercher Squat", ExerciseKind::Strength, 120);

    const QJsonObject json = WorkoutJson::exerciseToJson(exercise);

    EXPECT_FALSE(json.contains("definition_id"));
    EXPECT_FALSE(WorkoutJson::exerciseFromJson(json, nullptr).hasDefinition());
}

TEST(WorkoutJsonCatalogLinkTest, TheCompactFormStaysFreeOfCatalogIds)
{
    Exercise exercise
        = Exercise::createFromDefinition(42, "Bench Press", ExerciseKind::Strength, 180);
    exercise.addSet(Set(5, 100.0));

    const QJsonObject json = WorkoutJson::exerciseToJsonCompact(exercise);

    EXPECT_FALSE(json.contains("definition_id"));
    EXPECT_FALSE(json.contains("id"));
}

TEST(WorkoutJsonCatalogLinkTest, AWholeWorkoutKeepsItsLinksThroughTheCache)
{
    Workout workout("Full Body", QDateTime::currentDateTime());

    Exercise linked = Exercise::createFromDefinition(7, "Front Squat", ExerciseKind::Strength, 180);
    linked.addSet(Set(5, 90.0));
    workout.addExercise(linked);

    Exercise adHoc = Exercise::createAdHoc("Cable Crunch", ExerciseKind::Strength, 60);
    adHoc.addSet(Set(12, 30.0));
    workout.addExercise(adHoc);

    const Workout restored
        = WorkoutJson::workoutFromJson(WorkoutJson::workoutToJson(workout), nullptr);

    ASSERT_EQ(restored.exercises().size(), 2u);
    EXPECT_EQ(restored.exercises()[0].definitionId().value(), 7);
    EXPECT_FALSE(restored.exercises()[1].hasDefinition());
}
