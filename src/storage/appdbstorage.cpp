#include "appdbstorage.h"
#include <QDebug>
#include <QSqlError>

AppDbStorage::AppDbStorage(const QString &dbPath, QObject *parent)
    : QObject(parent)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);

    if (!m_database.open()) {
        qWarning() << "Failed to open database:" << m_database.lastError().text();
        return;
    }

    QSqlQuery pragmaQuery(m_database);
    if (!pragmaQuery.exec("PRAGMA foreign_keys = ON;")) {
        qWarning() << "Failed to enable foreign keys:" << pragmaQuery.lastError().text();
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

int AppDbStorage::saveWorkout(WorkoutModel *workout)
{
    if (!workout)
        return -1;

    workout->setCreatedTime(QDateTime::currentDateTime());
    int workoutId = m_workoutRepo->save(workout);
    workout->setId(workoutId);

    for (QObject *exerciseObj : workout->exercises()) {
        ExerciseModel *exercise = qobject_cast<ExerciseModel *>(exerciseObj);
        if (exercise) {
            exercise->setWorkoutId(workoutId);
            int exerciseId = m_exerciseRepo->save(exercise);
            exercise->setId(exerciseId);

            for (QObject *setObj : exercise->sets()) {
                SetModel *set = qobject_cast<SetModel *>(setObj);
                if (set) {
                    set->setExerciseId(exerciseId);
                    m_setRepo->save(set);
                }
            }
        }
    }

    return workoutId;
}
