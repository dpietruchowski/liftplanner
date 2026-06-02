#include "appdbstorage.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "modules/userprofile/infrastructure/database/userprofilerepositorydb.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>

AppDbStorage::AppDbStorage(const QString& dbPath, QObject* parent)
    : QObject(parent)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);

    if (!m_database.open())
    {
        qWarning() << "Failed to open database:" << m_database.lastError().text();
        return;
    }

    QSqlQuery pragmaQuery(m_database);
    if (!pragmaQuery.exec("PRAGMA foreign_keys = ON;"))
    {
        qWarning() << "Failed to enable foreign keys:" << pragmaQuery.lastError().text();
    }

    initializeDatabase();
}

AppDbStorage::~AppDbStorage() = default;

WorkoutRepositoryDb& AppDbStorage::workoutRepo() { return *m_workoutRepo; }

UserProfileRepositoryDb& AppDbStorage::userProfileRepo() { return *m_userProfileRepo; }

void AppDbStorage::initializeDatabase()
{
    m_dbStorage = std::make_unique<DbStorage>(m_database);
    m_workoutRepo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
    m_workoutRepo->createTables();
    m_userProfileRepo = std::make_unique<UserProfileRepositoryDb>(*m_dbStorage);
    m_userProfileRepo->createTable();

    MigrationRunner runner(*m_dbStorage);
    m_workoutRepo->registerMigrations(runner);
    m_userProfileRepo->registerMigrations(runner);
    runner.run();
}
