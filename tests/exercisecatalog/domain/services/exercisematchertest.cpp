#include "domain/exercisecatalog/exercisematcher.h"
#include <gtest/gtest.h>

namespace
{

std::vector<ExerciseMatcher::Candidate> catalog()
{
    return { { 1, "Barbell Back Squat", { "Back Squat", "Przysiad ze sztangą" } },
             { 2, "Flat Barbell Bench Press", { "Bench Press" } },
             { 3, "Overhead Press", {} },
             { 4, "Outdoor Run", {} } };
}

}

class ExerciseMatcherTest : public ::testing::Test
{
};

TEST_F(ExerciseMatcherTest, ExactNameWins)
{
    EXPECT_EQ(ExerciseMatcher::match("Overhead Press", catalog()).value(), 3);
}

TEST_F(ExerciseMatcherTest, NameMatchIsCaseInsensitive)
{
    EXPECT_EQ(ExerciseMatcher::match("overhead press", catalog()).value(), 3);
    EXPECT_EQ(ExerciseMatcher::match("OVERHEAD PRESS", catalog()).value(), 3);
}

TEST_F(ExerciseMatcherTest, AliasMatchesWhenTheNameDoesNot)
{
    EXPECT_EQ(ExerciseMatcher::match("Back Squat", catalog()).value(), 1);
    EXPECT_EQ(ExerciseMatcher::match("Bench Press", catalog()).value(), 2);
}

TEST_F(ExerciseMatcherTest, AliasMatchIgnoresAccentsAndCase)
{
    EXPECT_EQ(ExerciseMatcher::match("przysiad ze sztanga", catalog()).value(), 1);
}

TEST_F(ExerciseMatcherTest, PunctuationAndSpacingAreNormalizedAway)
{
    EXPECT_EQ(ExerciseMatcher::match("Overhead-Press", catalog()).value(), 3);
    EXPECT_EQ(ExerciseMatcher::match("  Overhead   Press  ", catalog()).value(), 3);
    EXPECT_EQ(ExerciseMatcher::match("Overhead Press!", catalog()).value(), 3);
}

TEST_F(ExerciseMatcherTest, AnUnknownNameDoesNotMatch)
{
    EXPECT_FALSE(ExerciseMatcher::match("Zercher Squat", catalog()).has_value());
}

TEST_F(ExerciseMatcherTest, AnEmptyNameDoesNotMatch)
{
    EXPECT_FALSE(ExerciseMatcher::match("", catalog()).has_value());
    EXPECT_FALSE(ExerciseMatcher::match("   ", catalog()).has_value());
}

TEST_F(ExerciseMatcherTest, AnEmptyCatalogDoesNotMatch)
{
    EXPECT_FALSE(ExerciseMatcher::match("Overhead Press", {}).has_value());
}

TEST_F(ExerciseMatcherTest, AmbiguityIsReportedAsNoMatchRatherThanAGuess)
{
    const std::vector<ExerciseMatcher::Candidate> ambiguous
        = { { 1, "Bench Press", {} }, { 2, "bench press", {} } };

    EXPECT_FALSE(ExerciseMatcher::match("Bench Press", ambiguous).has_value());
}

TEST_F(ExerciseMatcherTest, AmbiguityAtTheNormalizedTierIsAlsoNoMatch)
{
    const std::vector<ExerciseMatcher::Candidate> ambiguous
        = { { 1, "Bench-Press", {} }, { 2, "Bench.Press", {} } };

    ASSERT_EQ(ExerciseMatcher::normalize("Bench-Press"), ExerciseMatcher::normalize("Bench.Press"));
    EXPECT_FALSE(ExerciseMatcher::match("bench press", ambiguous).has_value());
    EXPECT_EQ(ExerciseMatcher::match("bench press", { ambiguous[0] }).value(), 1);
}

TEST_F(ExerciseMatcherTest, AnExactNameBeatsAnAliasOnAnotherCandidate)
{
    const std::vector<ExerciseMatcher::Candidate> candidates
        = { { 1, "Overhead Press", {} }, { 2, "Military Press", { "Overhead Press" } } };

    EXPECT_EQ(ExerciseMatcher::match("Overhead Press", candidates).value(), 1);
}

TEST_F(ExerciseMatcherTest, AnAliasBeatsANormalizedNameOnAnotherCandidate)
{
    const std::vector<ExerciseMatcher::Candidate> candidates
        = { { 1, "Barbell Row", { "Bent Over Row" } }, { 2, "Bent-Over-Row", {} } };

    EXPECT_EQ(ExerciseMatcher::match("Bent Over Row", candidates).value(), 1);
}

class ExerciseMatcherNormalizeTest : public ::testing::Test
{
};

TEST_F(ExerciseMatcherNormalizeTest, ReducesToLowerCaseWordsSeparatedBySingleSpaces)
{
    EXPECT_EQ(ExerciseMatcher::normalize("Barbell  Back-Squat!"), "barbell back squat");
    EXPECT_EQ(ExerciseMatcher::normalize("  Bench Press  "), "bench press");
    EXPECT_EQ(ExerciseMatcher::normalize("Przysiad ze sztangą"), "przysiad ze sztanga");
    EXPECT_EQ(ExerciseMatcher::normalize(""), "");
}

class ExerciseMatcherCandidateTest : public ::testing::Test
{
};

TEST_F(ExerciseMatcherCandidateTest, DefinitionsCollapseIntoLightweightCandidates)
{
    ExerciseDefinition squat("barbell-back-squat", "Barbell Back Squat", ExerciseKind::Strength);
    squat.setId(11);
    squat.addAlias("Back Squat");

    const std::vector<ExerciseMatcher::Candidate> candidates
        = ExerciseMatcher::toCandidates({ squat });

    ASSERT_EQ(candidates.size(), 1u);
    EXPECT_EQ(candidates[0].id, 11);
    EXPECT_EQ(candidates[0].name, "Barbell Back Squat");
    EXPECT_EQ(candidates[0].aliases.size(), 1);
    EXPECT_EQ(ExerciseMatcher::match("Back Squat", candidates).value(), 11);
}
