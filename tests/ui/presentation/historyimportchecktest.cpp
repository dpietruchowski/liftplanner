#include "ui/presentation/historyimportcheck.h"

#include <gtest/gtest.h>

TEST(HistoryImportCheckTest, AnEmptyClipboardIsNamedAsSuch)
{
    const QString problem = HistoryImportCheck::clipboardProblem(QString());

    EXPECT_FALSE(problem.isEmpty());
    EXPECT_TRUE(problem.contains(QStringLiteral("clipboard is empty")));
}

TEST(HistoryImportCheckTest, AClipboardHoldingOnlyWhitespaceCountsAsEmpty)
{
    EXPECT_FALSE(HistoryImportCheck::clipboardProblem(QStringLiteral("   \n\t ")).isEmpty());
}

TEST(HistoryImportCheckTest, AClipboardWithContentRaisesNoClipboardProblem)
{
    EXPECT_TRUE(HistoryImportCheck::clipboardProblem(QStringLiteral("[]")).isEmpty());
}

TEST(HistoryImportCheckTest, GarbageIsReportedAsBrokenJsonNotAsAnEmptyClipboard)
{
    const QString problem = HistoryImportCheck::payloadProblem(QStringLiteral("this is not json"));

    ASSERT_FALSE(problem.isEmpty());
    EXPECT_TRUE(problem.contains(QStringLiteral("valid JSON")));
    EXPECT_FALSE(problem.contains(QStringLiteral("clipboard is empty")));
}

TEST(HistoryImportCheckTest, TheParserReasonIsPartOfTheBrokenJsonMessage)
{
    const QString problem = HistoryImportCheck::payloadProblem(QStringLiteral("[{\"name\":}]"));

    ASSERT_FALSE(problem.isEmpty());
    EXPECT_GT(problem.size(), QStringLiteral("Import failed: ").size());
    EXPECT_TRUE(problem.endsWith(QStringLiteral("from your AI chat.")));
}

TEST(HistoryImportCheckTest, AJsonObjectIsNotAHistoryArray)
{
    const QString problem
        = HistoryImportCheck::payloadProblem(QStringLiteral("{\"workouts\": []}"));

    ASSERT_FALSE(problem.isEmpty());
    EXPECT_TRUE(problem.contains(QStringLiteral("JSON array of workouts")));
}

TEST(HistoryImportCheckTest, AnEmptyArrayHasNothingToImport)
{
    const QString problem = HistoryImportCheck::payloadProblem(QStringLiteral("[]"));

    ASSERT_FALSE(problem.isEmpty());
    EXPECT_TRUE(problem.contains(QStringLiteral("no workouts")));
}

TEST(HistoryImportCheckTest, AnEntryWithoutANameIsPointedAtByPosition)
{
    const QString problem = HistoryImportCheck::payloadProblem(
        QStringLiteral("[{\"name\": \"Push Day\"}, {\"sets\": 3}]"));

    ASSERT_FALSE(problem.isEmpty());
    EXPECT_TRUE(problem.contains(QStringLiteral("entry 2")));
}

TEST(HistoryImportCheckTest, ANumberInsteadOfAWorkoutIsRejected)
{
    EXPECT_FALSE(HistoryImportCheck::payloadProblem(QStringLiteral("[1, 2, 3]")).isEmpty());
}

TEST(HistoryImportCheckTest, AProperHistoryArrayPassesWithoutAWord)
{
    const QString payload = QStringLiteral(
        "[{\"name\": \"Push Day\", \"status\": \"Ended\", \"exercises\": ["
        "{\"name\": \"Bench Press\", \"sets\": [{\"repetitions\": 5, \"weight\": 60}]}]}]");

    EXPECT_TRUE(HistoryImportCheck::payloadProblem(payload).isEmpty());
}
