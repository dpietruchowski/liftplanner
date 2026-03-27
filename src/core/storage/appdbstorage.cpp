#include "appdbstorage.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include <dbtoolkit/dbstorage.h>
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "modules/workout/infrastructure/database/exerciserepositorydb.h"
#include "modules/workout/infrastructure/database/setrepositorydb.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/domain/repositories/exercisequery.h"
#include "modules/workout/domain/repositories/setquery.h"

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

ExerciseRepositoryDb &AppDbStorage::exerciseRepo()
{
    return *m_exerciseRepo;
}

SetRepositoryDb &AppDbStorage::setRepo()
{
    return *m_setRepo;
}

void AppDbStorage::initializeDatabase()
{
    m_dbStorage = std::make_unique<DbStorage>(m_database);
    m_workoutRepo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
    m_exerciseRepo = std::make_unique<ExerciseRepositoryDb>(*m_dbStorage);
    m_setRepo = std::make_unique<SetRepositoryDb>(*m_dbStorage);

    m_workoutRepo->createTable();
    m_exerciseRepo->createTable();
    m_setRepo->createTable();
}

std::vector<Workout> AppDbStorage::loadWorkouts(const WorkoutQuery &query)
{
    auto workoutEntities = m_workoutRepo->findAll(query);

    std::vector<Workout> result;
    result.reserve(workoutEntities.size());
    for (auto &w : workoutEntities)
    {
        auto exercises = m_exerciseRepo->findAll(
            ExerciseQuery().whereWorkoutId(w.id()));

        for (auto &e : exercises)
        {
            auto sets = m_setRepo->findAll(
                SetQuery().whereExerciseId(e.id()));

            for (const auto &s : sets)
                e.addSet(s);

            w.addExercise(e);
        }

        result.push_back(std::move(w));
    }
    return result;
}

Workout AppDbStorage::loadFullWorkout(int workoutId)
{
    WorkoutQuery query;
    query.whereId(workoutId);
    auto workouts = loadWorkouts(query);
    if (workouts.empty())
        return {};
    return workouts.front();
}

int AppDbStorage::saveWorkoutEntity(const Workout &workout)
{
    Workout w = workout;
    int workoutId = m_workoutRepo->save(w);
    w.setId(workoutId);

    for (const auto &exercise : workout.exercises())
    {
        Exercise e = exercise;
        e.setWorkoutId(workoutId);
        int exerciseId = m_exerciseRepo->save(e);
        e.setId(exerciseId);

        for (const auto &set : exercise.sets())
        {
            Set s = set;
            s.setExerciseId(exerciseId);
            m_setRepo->save(s);
        }
    }

    return workoutId;
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
