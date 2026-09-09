#include "appdbstorage.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "infrastructure/exercisecatalog/exercisecatalogseed.h"
#include "infrastructure/exercisecatalog/exercisedefinitionrepositorydb.h"
#include "infrastructure/exercisecatalog/exercisehistorybackfill.h"
#include "infrastructure/userprofile/userprofilerepositorydb.h"
#include "infrastructure/workout/workoutrepositorydb.h"
#include "infrastructure/workout/workouttemplaterepositorydb.h"
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>

AppDbStorage::AppDbStorage(const QString& dbPath, QObject* parent)
    : QObject(parent)
{
    m_database = std::make_unique<QSqlDatabase>(
        QSqlDatabase::addDatabase("QSQLITE", "liftplanner_connection"));
    m_database->setDatabaseName(dbPath);
}

AppDbStorage::~AppDbStorage() = default;

bool AppDbStorage::open()
{
    if (!m_database->open())
    {
        qWarning() << "Failed to open database:" << m_database->lastError().text();
        return false;
    }

    QSqlQuery pragmaQuery(*m_database);
    if (!pragmaQuery.exec("PRAGMA foreign_keys = ON;"))
    {
        qWarning() << "Failed to enable foreign keys:" << pragmaQuery.lastError().text();
    }

    m_dbStorage = std::make_unique<DbStorage>(*m_database);
    m_workoutRepo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
    m_workoutRepo->createTables();
    m_workoutTemplateRepo = std::make_unique<WorkoutTemplateRepositoryDb>(*m_dbStorage);
    m_workoutTemplateRepo->createTables();
    m_userProfileRepo = std::make_unique<UserProfileRepositoryDb>(*m_dbStorage);
    m_userProfileRepo->createTable();
    m_exerciseDefinitionRepo = std::make_unique<ExerciseDefinitionRepositoryDb>(*m_dbStorage);
    m_exerciseDefinitionRepo->createTables();

    MigrationRunner runner(*m_dbStorage);
    m_workoutRepo->registerMigrations(runner);
    m_workoutTemplateRepo->registerMigrations(runner);
    m_userProfileRepo->registerMigrations(runner);
    m_exerciseDefinitionRepo->registerMigrations(runner);
    runner.run();

    seedExerciseCatalog();
    backfillExerciseHistory();

    return true;
}

void AppDbStorage::backfillExerciseHistory()
{
    const HistoryBackfillResult result
        = ExerciseHistoryBackfill::apply(*m_database, *m_exerciseDefinitionRepo);

    if (result.examined == 0)
        return;

    qInfo() << "Exercise history backfill: examined" << result.examined << "linked" << result.linked
            << "unmatched" << result.unmatched;
}

void AppDbStorage::seedExerciseCatalog()
{
    const CatalogSeedResult result = ExerciseCatalogSeed::apply(*m_exerciseDefinitionRepo);

    for (const QString& error : result.errors)
        qWarning() << "Exercise catalog seed:" << error;

    qInfo() << "Exercise catalog seed: inserted" << result.inserted << "updated" << result.updated
            << "archived" << result.archived;
}

WorkoutRepositoryDb& AppDbStorage::workoutRepo() { return *m_workoutRepo; }

WorkoutTemplateRepositoryDb& AppDbStorage::workoutTemplateRepo() { return *m_workoutTemplateRepo; }

UserProfileRepositoryDb& AppDbStorage::userProfileRepo() { return *m_userProfileRepo; }

ExerciseDefinitionRepositoryDb& AppDbStorage::exerciseDefinitionRepo()
{
    return *m_exerciseDefinitionRepo;
}
