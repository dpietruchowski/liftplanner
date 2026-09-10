#include "domain/workout/performedsets.h"
#include <gtest/gtest.h>

namespace
{

Set tickedSet(int repetitions, double weight)
{
    Set set(repetitions, weight);
    set.setCompleted(true);
    return set;
}

Exercise exerciseWith(const QString& name, std::initializer_list<Set> sets)
{
    Exercise exercise(name, 120);
    for (const Set& set : sets)
        exercise.addSet(set);
    return exercise;
}

Workout sessionWith(std::initializer_list<Exercise> exercises)
{
    Workout workout(QStringLiteral("Base Strength"), QDateTime(QDate(2025, 1, 2), QTime(18, 0, 0)));
    for (const Exercise& exercise : exercises)
        workout.addExercise(exercise);
    return workout;
}

HistorySetRow setRow(int workoutId, bool ticked)
{
    HistorySetRow row;
    row.workoutId = workoutId;
    row.hasSet = true;
    row.completed = ticked;
    return row;
}

HistorySetRow emptySessionRow(int workoutId)
{
    HistorySetRow row;
    row.workoutId = workoutId;
    return row;
}

}  // namespace

class PerformedSetsTest : public ::testing::Test
{
};

TEST_F(PerformedSetsTest, TheDecisionIsTheSameWhicheverShapeTheDataHas)
{
    const Exercise untickedExercise = exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0) });
    const Exercise tickedExercise
        = exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 45.0) });

    EXPECT_EQ(wasPerformed(untickedExercise, false), wasPerformed(setRow(1, false), false));
    EXPECT_EQ(wasPerformed(untickedExercise, true), wasPerformed(setRow(1, false), true));
    EXPECT_EQ(wasPerformed(tickedExercise, false), wasPerformed(setRow(1, true), false));
    EXPECT_EQ(wasPerformed(tickedExercise, true), wasPerformed(setRow(1, true), true));
}

TEST_F(PerformedSetsTest, RowsOfASessionWithoutASingleTickCarryNoMeaningfulFlag)
{
    const std::vector<HistorySetRow> rows = { setRow(1, false), setRow(1, false), setRow(2, true) };

    const QSet<int> meaningful = sessionsWithMeaningfulFlags(rows);

    EXPECT_FALSE(meaningful.contains(1));
    EXPECT_TRUE(meaningful.contains(2));
}

TEST_F(PerformedSetsTest, ASessionWithoutAnySetsNeverHasMeaningfulFlags)
{
    const QSet<int> meaningful = sessionsWithMeaningfulFlags({ emptySessionRow(1) });

    EXPECT_TRUE(meaningful.isEmpty());
}

TEST_F(PerformedSetsTest, ARowWithoutASetNeverCounts)
{
    EXPECT_FALSE(wasPerformed(emptySessionRow(1), false));
    EXPECT_FALSE(wasPerformed(emptySessionRow(1), true));
}

TEST_F(PerformedSetsTest, WithoutMeaningfulFlagsEveryRowWithASetCounts)
{
    EXPECT_TRUE(wasPerformed(setRow(1, false), false));
    EXPECT_TRUE(wasPerformed(setRow(1, true), false));
}

TEST_F(PerformedSetsTest, WithMeaningfulFlagsOnlyTickedRowsCount)
{
    EXPECT_FALSE(wasPerformed(setRow(1, false), true));
    EXPECT_TRUE(wasPerformed(setRow(1, true), true));
}

TEST_F(PerformedSetsTest, ASessionWithNoTicksAnywhereCarriesNoInformationInTheFlag)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0), Set(5, 45.0) }),
                        exerciseWith(QStringLiteral("Row"), { Set(8, 40.0) }) });

    EXPECT_FALSE(completionFlagsAreMeaningful(session));
}

TEST_F(PerformedSetsTest, ASingleTickAnywhereMakesTheFlagMeaningfulForTheWholeSession)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0) }),
                        exerciseWith(QStringLiteral("Row"), { tickedSet(8, 40.0) }) });

    EXPECT_TRUE(completionFlagsAreMeaningful(session));
}

TEST_F(PerformedSetsTest, AnEmptySessionHasNoMeaningfulFlags)
{
    EXPECT_FALSE(completionFlagsAreMeaningful(sessionWith({})));
}

TEST_F(PerformedSetsTest, WithoutMeaningfulFlagsEverySetWithContentCounts)
{
    const Exercise bench = exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0) });

    EXPECT_TRUE(wasPerformed(bench, false));
    EXPECT_FALSE(wasPerformed(bench, true));
}

TEST_F(PerformedSetsTest, AnExerciseWithoutSetsIsNeverPerformed)
{
    const Exercise empty = exerciseWith(QStringLiteral("Bench Press"), {});

    EXPECT_FALSE(wasPerformed(empty, false));
    EXPECT_FALSE(wasPerformed(empty, true));
}

TEST_F(PerformedSetsTest, WithMeaningfulFlagsOnlyTickedExercisesCount)
{
    const Exercise skipped = exerciseWith(QStringLiteral("Row"), { Set(8, 40.0) });
    const Exercise done = exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 45.0) });

    EXPECT_FALSE(wasPerformed(skipped, true));
    EXPECT_TRUE(wasPerformed(done, true));
}

TEST_F(PerformedSetsTest, WithoutMeaningfulFlagsEverySetIsReportedAsPerformed)
{
    const Exercise bench
        = exerciseWith(QStringLiteral("Bench Press"), { Set(5, 20.0), Set(5, 30.0), Set(5, 45.0) });

    const Exercise performed = asPerformed(bench, false);

    ASSERT_EQ(performed.sets().size(), 3u);
    for (const Set& set : performed.sets())
        EXPECT_TRUE(set.completed());
    EXPECT_DOUBLE_EQ(performed.sets()[2].weight(), 45.0);
}

TEST_F(PerformedSetsTest, WithMeaningfulFlagsTheExerciseIsReportedUntouched)
{
    const Exercise bench
        = exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 20.0), Set(5, 45.0) });

    const Exercise performed = asPerformed(bench, true);

    ASSERT_EQ(performed.sets().size(), 2u);
    EXPECT_TRUE(performed.sets()[0].completed());
    EXPECT_FALSE(performed.sets()[1].completed());
}

TEST_F(PerformedSetsTest, TrimmingDropsTheUntickedSetsOfATickedExercise)
{
    const Workout session = sessionWith({ exerciseWith(
        QStringLiteral("Bench Press"), { tickedSet(5, 45.0), Set(5, 45.0), tickedSet(5, 50.0) }) });

    const Workout trimmed = trimmedToPerformed(session);

    ASSERT_EQ(trimmed.exercises().size(), 1u);
    ASSERT_EQ(trimmed.exercises()[0].sets().size(), 2u);
    EXPECT_DOUBLE_EQ(trimmed.exercises()[0].sets()[0].weight(), 45.0);
    EXPECT_DOUBLE_EQ(trimmed.exercises()[0].sets()[1].weight(), 50.0);
}

TEST_F(PerformedSetsTest, TrimmingDropsAnExerciseWithoutASingleTickedSet)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 45.0) }),
                        exerciseWith(QStringLiteral("Row"), { Set(8, 40.0), Set(8, 40.0) }),
                        exerciseWith(QStringLiteral("Curl"), {}) });

    const Workout trimmed = trimmedToPerformed(session);

    ASSERT_EQ(trimmed.exercises().size(), 1u);
    EXPECT_EQ(trimmed.exercises()[0].name(), QStringLiteral("Bench Press"));
}

TEST_F(PerformedSetsTest, TrimmingRenumbersWhatIsLeft)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Row"), { Set(8, 40.0) }),
                        exerciseWith(QStringLiteral("Bench Press"),
                                     { Set(5, 45.0), tickedSet(5, 50.0), tickedSet(5, 55.0) }) });

    const Workout trimmed = trimmedToPerformed(session);

    ASSERT_EQ(trimmed.exercises().size(), 1u);
    EXPECT_EQ(trimmed.exercises()[0].position(), 0);
    ASSERT_EQ(trimmed.exercises()[0].sets().size(), 2u);
    EXPECT_EQ(trimmed.exercises()[0].sets()[0].position(), 0);
    EXPECT_EQ(trimmed.exercises()[0].sets()[1].position(), 1);
}

TEST_F(PerformedSetsTest, ASessionWithoutASingleTickIsKeptWholeByTrimming)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0), Set(5, 45.0) }),
                        exerciseWith(QStringLiteral("Row"), { Set(8, 40.0) }) });

    const Workout trimmed = trimmedToPerformed(session);

    ASSERT_EQ(trimmed.exercises().size(), 2u);
    EXPECT_EQ(trimmed.exercises()[0].sets().size(), 2u);
    EXPECT_EQ(trimmed.exercises()[1].sets().size(), 1u);
}

TEST_F(PerformedSetsTest, TrimmingKeepsTheSessionItselfUntouched)
{
    Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 45.0) }) });
    session.setId(172);
    session.setStatus(WorkoutStatus::Ended);

    const Workout trimmed = trimmedToPerformed(session);

    EXPECT_EQ(trimmed.id(), 172);
    EXPECT_EQ(trimmed.name(), QStringLiteral("Base Strength"));
    EXPECT_EQ(trimmed.status(), WorkoutStatus::Ended);
    EXPECT_EQ(trimmed.createdTime(), session.createdTime());
}

TEST_F(PerformedSetsTest, TrimmingKeepsTheIdentityOfTheSetsItKeeps)
{
    Set kept = tickedSet(5, 45.0);
    kept.setId(9001);
    Set dropped(5, 45.0);
    dropped.setId(9002);

    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { dropped, kept }) });

    const Workout trimmed = trimmedToPerformed(session);

    ASSERT_EQ(trimmed.exercises().size(), 1u);
    ASSERT_EQ(trimmed.exercises()[0].sets().size(), 1u);
    EXPECT_EQ(trimmed.exercises()[0].sets()[0].id(), 9001);
}

TEST_F(PerformedSetsTest, ATrimmedSessionCountsAsFullyPerformed)
{
    const Workout session
        = sessionWith({ exerciseWith(QStringLiteral("Bench Press"), { tickedSet(5, 45.0) }),
                        exerciseWith(QStringLiteral("Row"), { Set(8, 40.0) }) });

    const Workout trimmed = trimmedToPerformed(session);

    EXPECT_TRUE(trimmed.isCompleted());
    EXPECT_EQ(trimmed.totalSets(), 1);
}

TEST_F(PerformedSetsTest, ReportingAsPerformedKeepsTheValues)
{
    const Exercise bench = exerciseWith(QStringLiteral("Bench Press"), { Set(5, 45.0) });

    const Exercise performed = asPerformed(bench, false);

    ASSERT_EQ(performed.sets().size(), 1u);
    EXPECT_EQ(performed.sets()[0].repetitions(), 5);
    EXPECT_DOUBLE_EQ(performed.sets()[0].weight(), 45.0);
    EXPECT_EQ(performed.name(), QStringLiteral("Bench Press"));
}
