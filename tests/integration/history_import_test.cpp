#include <QSignalSpy>
#include <gtest/gtest.h>

#include "testapplication.h"
#include "ui/models/workoutmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"

class HistoryImportTest : public ::testing::Test
{
protected:
    WorkoutHistoryViewModel& history() { return m_app.workoutHistoryViewModel(); }

    QString importAndCatchError(const QString& payload)
    {
        QSignalSpy errors(&history(), &WorkoutHistoryViewModel::errorOccurred);
        history().importFromJson(payload);
        m_app.drain();

        if (errors.isEmpty())
            return QString();

        return errors.first().first().toString();
    }

    static QString validHistoryJson()
    {
        return QStringLiteral(
            "[{\"name\": \"Imported Session\", \"status\": \"Ended\","
            " \"started_time\": \"2025-01-03T18:00:00\","
            " \"ended_time\": \"2025-01-03T19:00:00\","
            " \"exercises\": [{\"name\": \"Bench Press\", \"rest_seconds\": 120,"
            " \"sets\": [{\"repetitions\": 5, \"weight\": 60, \"completed\": true}]}]}]");
    }

    TestApplication m_app;
};

TEST_F(HistoryImportTest, GarbageInsteadOfJsonIsReported)
{
    const QString error = importAndCatchError(QStringLiteral("nonsense from the chat window"));

    EXPECT_FALSE(error.isEmpty());
    EXPECT_TRUE(error.contains(QStringLiteral("valid JSON")));
}

TEST_F(HistoryImportTest, AnEmptyPayloadIsReported)
{
    EXPECT_FALSE(importAndCatchError(QString()).isEmpty());
}

TEST_F(HistoryImportTest, APlannedShapedObjectIsReportedAsTheWrongShape)
{
    const QString error
        = importAndCatchError(QStringLiteral("{\"workouts\": [{\"name\": \"Push Day\"}]}"));

    EXPECT_TRUE(error.contains(QStringLiteral("JSON array of workouts")));
}

TEST_F(HistoryImportTest, ARejectedImportLeavesTheHistoryUntouched)
{
    history().importFromJson(QStringLiteral("nonsense"));
    m_app.drain();

    EXPECT_TRUE(history().workouts().isEmpty());
}

TEST_F(HistoryImportTest, AValidHistoryImportSaysNothing)
{
    EXPECT_TRUE(importAndCatchError(validHistoryJson()).isEmpty());
}

TEST_F(HistoryImportTest, AValidHistoryImportLandsInTheHistory)
{
    history().importFromJson(validHistoryJson());
    m_app.drain();
    history().loadAllWorkouts();
    m_app.drain();

    ASSERT_EQ(history().workouts().size(), 1);
    EXPECT_EQ(history().workouts().first()->name(), QStringLiteral("Imported Session"));
}
