#include "appdbstorage.h"
#include <QDebug>
#include <QSqlError>

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

    initializeDatabase();
}

void AppDbStorage::initializeDatabase()
{
    m_workoutRepo = new WorkoutDbRepository(m_database, this);
    m_exerciseRepo = new ExerciseDbRepository(m_database, this);
    m_setRepo = new SetDbRepository(m_database, this);
}

WorkoutDbRepository *AppDbStorage::workoutRepository() { return m_workoutRepo; }
ExerciseDbRepository *AppDbStorage::exerciseRepository() { return m_exerciseRepo; }
SetDbRepository *AppDbStorage::setRepository()
{
    return m_setRepo;
}
