#include "appdbstorage.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include <dbtoolkit/dbstorage.h>
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"

AppDbStorage::AppDbStorage(const QString &dbPath, QObject *parent)
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

WorkoutRepositoryDb &AppDbStorage::workoutRepo()
{
    return *m_workoutRepo;
}

void AppDbStorage::initializeDatabase()
{
    m_dbStorage = std::make_unique<DbStorage>(m_database);
    m_workoutRepo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
    m_workoutRepo->createTables();
}

std::vector<Workout> AppDbStorage::loadWorkouts(const WorkoutQuery &query)
{
    return m_workoutRepo->findAll(query);
}

Workout AppDbStorage::loadFullWorkout(int workoutId)
{
    WorkoutQuery query;
    query.whereId(workoutId);
    auto workouts = m_workoutRepo->findAll(query);
    if (workouts.empty())
        return {};
    return workouts.front();
}

int AppDbStorage::saveWorkout(const Workout &workout)
{
    return m_workoutRepo->save(workout);
}

bool AppDbStorage::removeWorkout(int workoutId)
{
    WorkoutQuery query;
    query.whereId(workoutId);
    return m_workoutRepo->remove(query);
}

bool AppDbStorage::removeWorkoutsByQuery(const WorkoutQuery &query)
{
    return m_workoutRepo->remove(query);
}
