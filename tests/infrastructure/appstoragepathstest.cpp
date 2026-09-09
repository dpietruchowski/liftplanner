#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <gtest/gtest.h>

#include "infrastructure/appstoragepaths.h"

namespace
{
const char* dataHomeVariable = "XDG_DATA_HOME";
}

class AppStoragePathsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_dataHome.isValid());
        ASSERT_TRUE(m_workingDir.isValid());

        m_previousDataHome = qgetenv(dataHomeVariable);
        m_previousWorkingDir = QDir::currentPath();

        qputenv(dataHomeVariable, m_dataHome.path().toUtf8());
        ASSERT_TRUE(QDir::setCurrent(m_workingDir.path()));
    }

    void TearDown() override
    {
        QDir::setCurrent(m_previousWorkingDir);
        if (m_previousDataHome.isEmpty())
            qunsetenv(dataHomeVariable);
        else
            qputenv(dataHomeVariable, m_previousDataHome);
    }

    void writeFile(const QString& path, const QByteArray& content)
    {
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(file.write(content), content.size());
    }

    QTemporaryDir m_dataHome;
    QTemporaryDir m_workingDir;
    QByteArray m_previousDataHome;
    QString m_previousWorkingDir;
};

TEST_F(AppStoragePathsTest, TheDatabaseLivesUnderTheDataHomeNotTheWorkingDirectory)
{
    const QString path = AppStoragePaths::database(QStringLiteral("liftplanner.db"));

    EXPECT_TRUE(path.startsWith(m_dataHome.path()));
    EXPECT_FALSE(path.startsWith(m_workingDir.path()));
}

TEST_F(AppStoragePathsTest, ADatabaseNextToTheApplicationIsAdoptedOnce)
{
    writeFile(QStringLiteral("liftplanner.db"), QByteArrayLiteral("first"));

    const QString path = AppStoragePaths::database(QStringLiteral("liftplanner.db"));

    QFile adopted(path);
    ASSERT_TRUE(adopted.open(QIODevice::ReadOnly));
    EXPECT_EQ(adopted.readAll(), QByteArrayLiteral("first"));
}

TEST_F(AppStoragePathsTest, AdoptionLeavesTheOriginalWhereItWas)
{
    writeFile(QStringLiteral("liftplanner.db"), QByteArrayLiteral("first"));

    AppStoragePaths::database(QStringLiteral("liftplanner.db"));

    QFile original(QStringLiteral("liftplanner.db"));
    ASSERT_TRUE(original.open(QIODevice::ReadOnly));
    EXPECT_EQ(original.readAll(), QByteArrayLiteral("first"));
}

TEST_F(AppStoragePathsTest, AnAdoptedDatabaseIsNeverOverwrittenOnTheNextStart)
{
    writeFile(QStringLiteral("liftplanner.db"), QByteArrayLiteral("first"));
    const QString path = AppStoragePaths::database(QStringLiteral("liftplanner.db"));
    writeFile(path, QByteArrayLiteral("later work"));
    writeFile(QStringLiteral("liftplanner.db"), QByteArrayLiteral("stale"));

    EXPECT_EQ(AppStoragePaths::database(QStringLiteral("liftplanner.db")), path);

    QFile kept(path);
    ASSERT_TRUE(kept.open(QIODevice::ReadOnly));
    EXPECT_EQ(kept.readAll(), QByteArrayLiteral("later work"));
}

TEST_F(AppStoragePathsTest, WithNothingToAdoptThePathIsStillTheOneUnderTheDataHome)
{
    const QString path = AppStoragePaths::database(QStringLiteral("liftplanner.db"));

    EXPECT_FALSE(QFile::exists(path));
    EXPECT_EQ(path, AppStoragePaths::file(QStringLiteral("liftplanner.db")));
}
