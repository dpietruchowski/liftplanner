#include "ui/presentation/plural.h"
#include "ui/presentation/workouttext.h"

#include <gtest/gtest.h>

namespace
{
const QString set = QStringLiteral("set");
const QString sets = QStringLiteral("sets");
}

TEST(PluralTest, OneTakesTheSingular)
{
    EXPECT_EQ(Plural::form(1, set, sets), set);
    EXPECT_EQ(Plural::counted(1, set, sets), QStringLiteral("1 set"));
}

TEST(PluralTest, EveryOtherCountTakesThePlural)
{
    EXPECT_EQ(Plural::counted(0, set, sets), QStringLiteral("0 sets"));
    EXPECT_EQ(Plural::counted(2, set, sets), QStringLiteral("2 sets"));
    EXPECT_EQ(Plural::counted(22, set, sets), QStringLiteral("22 sets"));
}

TEST(PluralTest, TheRuleCarriesVerbsAsWellAsNouns)
{
    EXPECT_EQ(Plural::form(1, QStringLiteral("is"), QStringLiteral("are")), QStringLiteral("is"));
    EXPECT_EQ(Plural::form(3, QStringLiteral("is"), QStringLiteral("are")), QStringLiteral("are"));
}

TEST(PluralTest, TheQmlSingletonAnswersTheSameWay)
{
    PluralText plural;

    EXPECT_EQ(plural.counted(1, set, sets), QStringLiteral("1 set"));
    EXPECT_EQ(plural.form(4, set, sets), sets);
}

TEST(FinishPromptTest, OneTickedSetOutOfManyUsesTheSingularVerb)
{
    EXPECT_EQ(WorkoutText::finishPrompt(1, 22),
              QStringLiteral(
                  "1 of 22 sets is ticked off. Ending now deletes the other 21 sets for good."));
}

TEST(FinishPromptTest, BothNumbersInTheSentenceBendOnTheirOwn)
{
    EXPECT_EQ(
        WorkoutText::finishPrompt(1, 2),
        QStringLiteral("1 of 2 sets is ticked off. Ending now deletes the other 1 set for good."));
}

TEST(FinishPromptTest, SeveralTickedSetsUseThePluralVerb)
{
    EXPECT_EQ(WorkoutText::finishPrompt(3, 22),
              QStringLiteral(
                  "3 of 22 sets are ticked off. Ending now deletes the other 19 sets for good."));
}

TEST(FinishPromptTest, ThePromptNamesHowManySetsGoAndThatTheyGoForGood)
{
    const QString prompt = WorkoutText::finishPrompt(2, 24);

    EXPECT_TRUE(prompt.contains(QStringLiteral("22 sets")));
    EXPECT_TRUE(prompt.contains(QStringLiteral("deletes")));
    EXPECT_TRUE(prompt.contains(QStringLiteral("for good")));
}

TEST(FinishPromptTest, AFullyTickedWorkoutCountsItsSets)
{
    EXPECT_EQ(WorkoutText::finishPrompt(22, 22),
              QStringLiteral("All 22 sets are ticked off. End this workout?"));
}

TEST(FinishPromptTest, ASingleSetWorkoutIsNotForcedIntoThePlural)
{
    EXPECT_EQ(WorkoutText::finishPrompt(1, 1),
              QStringLiteral("All 1 set is ticked off. End this workout?"));
}
