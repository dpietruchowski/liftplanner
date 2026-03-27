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
#include "models/workoutmodel.h"
#include "models/exercisemodel.h"
#include "models/setmodel.h"

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

Workout AppDbStorage::workoutFromModel(const WorkoutModel *model)
{
    Workout w;
    w.setId(model->id());
    w.setName(model->name());
    w.setCreatedTime(model->createdTime());
    w.setStartedTime(model->startedTime());
    w.setEndedTime(model->endedTime());
    return w;
}

Exercise AppDbStorage::exerciseFromModel(const ExerciseModel *model)
{
    Exercise e;
    e.setId(model->id());
    e.setWorkoutId(model->workoutId());
    e.setName(model->name());
    e.setDescription(model->description());
    e.setRestSeconds(model->restSeconds());
    e.setYoutubeLink(model->youtubeLink());
    return e;
}

Set AppDbStorage::setFromModel(const SetModel *model)
{
    Set s;
    s.setId(model->id());
    s.setExerciseId(model->exerciseId());
    s.setRepetitions(model->repetitions());
    s.setWeight(model->weight());
    s.setCompleted(model->completed());
    return s;
}

void AppDbStorage::populateWorkoutModel(WorkoutModel *model, const Workout &entity)
{
    model->setId(entity.id());
    model->setName(entity.name());
    model->setCreatedTime(entity.createdTime());
    model->setStartedTime(entity.startedTime());
    model->setEndedTime(entity.endedTime());
}

void AppDbStorage::populateExerciseModel(ExerciseModel *model, const Exercise &entity)
{
    model->setId(entity.id());
    model->setWorkoutId(entity.workoutId());
    model->setName(entity.name());
    model->setDescription(entity.description());
    model->setRestSeconds(entity.restSeconds());
    model->setYoutubeLink(entity.youtubeLink());
}

void AppDbStorage::populateSetModel(SetModel *model, const Set &entity)
{
    model->setId(entity.id());
    model->setExerciseId(entity.exerciseId());
    model->setRepetitions(entity.repetitions());
    model->setWeight(entity.weight());
    model->setCompleted(entity.completed());
}

int AppDbStorage::saveWorkout(WorkoutModel *workout)
{
    if (!workout)
        return -1;

    workout->setCreatedTime(QDateTime::currentDateTime());

    Workout workoutEntity = workoutFromModel(workout);
    int workoutId = m_workoutRepo->save(workoutEntity);
    workout->setId(workoutId);

    for (ExerciseModel *exercise : workout->exercises())
    {
        if (exercise)
        {
            exercise->setWorkoutId(workoutId);

            Exercise exerciseEntity = exerciseFromModel(exercise);
            int exerciseId = m_exerciseRepo->save(exerciseEntity);
            exercise->setId(exerciseId);

            for (QObject *setObj : exercise->sets())
            {
                SetModel *set = qobject_cast<SetModel *>(setObj);
                if (set)
                {
                    set->setExerciseId(exerciseId);
                    Set setEntity = setFromModel(set);
                    int setId = m_setRepo->save(setEntity);
                    set->setId(setId);
                }
            }
        }
    }

    return workoutId;
}

void AppDbStorage::loadWorkout(WorkoutModel *workout)
{
    if (!workout)
        return;

    auto exercises = m_exerciseRepo->findAll(
        ExerciseQuery().whereWorkoutId(workout->id()));

    for (const auto &exerciseEntity : exercises)
    {
        auto *exerciseModel = new ExerciseModel(workout);
        populateExerciseModel(exerciseModel, exerciseEntity);

        auto sets = m_setRepo->findAll(
            SetQuery().whereExerciseId(exerciseEntity.id()));

        for (const auto &setEntity : sets)
        {
            auto *setModel = new SetModel(exerciseModel);
            populateSetModel(setModel, setEntity);
            exerciseModel->addSet(setModel);
        }

        workout->addExercise(exerciseModel);
    }
}

QVector<WorkoutModel *> AppDbStorage::loadAllWorkouts(const QString &condition)
{
    WorkoutQuery query;

    if (condition.contains("started_time IS NULL"))
    {
        query.whereStartedTimeIsNull();
    }
    else if (condition.contains("started_time IS NOT NULL"))
    {
        query.whereStartedTimeIsNotNull();
        query.orderByStartedTime(SortDirection::Descending);
    }

    auto workouts = m_workoutRepo->findAll(query);

    QVector<WorkoutModel *> result;
    result.reserve(static_cast<int>(workouts.size()));
    for (const auto &entity : workouts)
    {
        auto *model = new WorkoutModel();
        populateWorkoutModel(model, entity);
        result.append(model);
    }
    return result;
}

bool AppDbStorage::removeWorkouts(const QString &condition)
{
    WorkoutQuery query;

    if (condition.contains("started_time IS NULL"))
    {
        query.whereStartedTimeIsNull();
    }
    else if (condition.contains("id ="))
    {
        QRegularExpression rx("id\\s*=\\s*(\\d+)");
        auto match = rx.match(condition);
        if (match.hasMatch())
        {
            query.whereId(match.captured(1).toInt());
        }
    }

    return m_workoutRepo->remove(query);
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
