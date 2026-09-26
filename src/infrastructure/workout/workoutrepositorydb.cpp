#include "workoutrepositorydb.h"
#include "domain/workout/exercise.h"
#include "domain/workout/set.h"
#include "domain/workout/workout.h"
#include "domain/workout/workoutquery.h"
#include "domain/workout/workoutstatus.h"
#include "infrastructure/dbrows.h"
#include "infrastructure/whereclause.h"
#include "infrastructure/workout/generatednamebackfill.h"
#include "infrastructure/workout/workoutserializer.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>
#include <dbtoolkit/query/altertable.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/update.h>
#include <dbtoolkit/query/where.h>

#include <QHash>

WorkoutRepositoryDb::WorkoutRepositoryDb(DbStorage& storage)
    : m_workoutRepo(std::make_unique<DbRepository>(
          WorkoutSerializer::table, WorkoutSerializer::id_key,
          QStringList { WorkoutSerializer::id_key, WorkoutSerializer::name_key,
                        WorkoutSerializer::created_time_key, WorkoutSerializer::planned_time_key,
                        WorkoutSerializer::started_time_key, WorkoutSerializer::ended_time_key,
                        WorkoutSerializer::status_key, WorkoutSerializer::generated_name_key },
          storage, nullptr))
    , m_exerciseRepo(storage)
    , m_setRepo(storage)
{
}

WorkoutRepositoryDb::~WorkoutRepositoryDb() = default;

bool WorkoutRepositoryDb::createTables()
{
    CreateTable workouts(WorkoutSerializer::table);
    workouts.ifNotExists()
        .column(Column(WorkoutSerializer::id_key).integer().primaryKey().autoIncrement().notNull())
        .column(Column(WorkoutSerializer::name_key).text())
        .column(Column(WorkoutSerializer::created_time_key).text())
        .column(Column(WorkoutSerializer::planned_time_key).text())
        .column(Column(WorkoutSerializer::started_time_key).text())
        .column(Column(WorkoutSerializer::ended_time_key).text())
        .column(Column(WorkoutSerializer::status_key).text())
        .column(Column(WorkoutSerializer::generated_name_key).integer().defaultValue(0));

    return m_workoutRepo->createTable(workouts) && m_exerciseRepo.createTable()
        && m_setRepo.createTable();
}

void WorkoutRepositoryDb::registerMigrations(MigrationRunner& runner)
{
    m_exerciseRepo.registerMigrations(runner);
    m_setRepo.registerMigrations(runner);

    runner.add(
        8,
        [](QSqlDatabase& db)
        {
            const QString planned = workoutStatusToString(WorkoutStatus::Planned);
            const bool columnAdded
                = AlterTable(WorkoutSerializer::table)
                      .addColumn(Column(WorkoutSerializer::status_key).text().defaultValue(planned))
                      .execute(db)
                      .toInt()
                != 0;

            const bool endedMarked
                = Update(WorkoutSerializer::table)
                      .set(WorkoutSerializer::status_key,
                           workoutStatusToString(WorkoutStatus::Ended))
                      .where(Where(WorkoutSerializer::status_key)
                                 .equals(planned)
                                 .and_(Where(WorkoutSerializer::ended_time_key).isNotNull()))
                      .execute(db)
                      .toInt()
                >= 0;

            const bool startedMarked
                = Update(WorkoutSerializer::table)
                      .set(WorkoutSerializer::status_key,
                           workoutStatusToString(WorkoutStatus::Started))
                      .where(Where(WorkoutSerializer::status_key)
                                 .equals(planned)
                                 .and_(Where(WorkoutSerializer::started_time_key).isNotNull())
                                 .and_(Where(WorkoutSerializer::ended_time_key).isNull()))
                      .execute(db)
                      .toInt()
                >= 0;

            return columnAdded && endedMarked && startedMarked;
        });

    runner.add(
        9,
        [](QSqlDatabase& db)
        {
            const bool columnAdded
                = AlterTable(WorkoutSerializer::table)
                      .addColumn(
                          Column(WorkoutSerializer::generated_name_key).integer().defaultValue(0))
                      .execute(db)
                      .toInt()
                != 0;

            return columnAdded && backfillGeneratedNames(db);
        });
}

std::vector<Workout> WorkoutRepositoryDb::findAll(const WorkoutQuery& query) const
{
    auto where = buildWhereClause(query);
    auto order = buildOrderClause(query);
    int limit = query.limit().value_or(-1);
    int offset = query.offset().value_or(-1);

    auto results = DbRows::toEntities(m_workoutRepo->select(where, order, limit, offset),
                                      WorkoutSerializer::fromVariant);

    loadChildren(results);
    return results;
}

std::optional<Workout> WorkoutRepositoryDb::findOne(const WorkoutQuery& query) const
{
    auto where = buildWhereClause(query);
    auto order = buildOrderClause(query);
    auto rows = m_workoutRepo->select(where, order, 1);
    if (rows.isEmpty())
        return std::nullopt;

    Workout w = WorkoutSerializer::fromVariant(rows.first());
    loadChildren(w);
    return w;
}

int WorkoutRepositoryDb::save(const Workout& workout)
{
    DbStorage& storage = m_workoutRepo->storage();
    storage.beginTransaction();

    const int workoutId = m_workoutRepo->upsert(WorkoutSerializer::toVariant(workout)).toInt();

    saveChildren(workoutId, workout);

    storage.commit();
    return workoutId;
}

bool WorkoutRepositoryDb::saveSet(const Set& set)
{
    if (set.id() == -1 || set.exerciseId() == -1)
        return false;

    return m_setRepo.save(set) == set.id();
}

bool WorkoutRepositoryDb::remove(const WorkoutQuery& query)
{
    auto where = buildWhereClause(query);
    return m_workoutRepo->remove(where) > 0;
}

int WorkoutRepositoryDb::count(const WorkoutQuery& query) const
{
    auto where = buildWhereClause(query);
    return m_workoutRepo->count(where);
}

bool WorkoutRepositoryDb::exists(const WorkoutQuery& query) const
{
    auto where = buildWhereClause(query);
    return m_workoutRepo->exists(where);
}

void WorkoutRepositoryDb::loadChildren(Workout& workout) const
{
    auto exercises = m_exerciseRepo.findByWorkoutId(workout.id());

    for (auto& exercise : exercises)
    {
        auto sets = m_setRepo.findByExerciseId(exercise.id());
        for (auto& set : sets)
            exercise.addSet(set);

        workout.addExercise(exercise);
    }
}

void WorkoutRepositoryDb::loadChildren(std::vector<Workout>& workouts) const
{
    if (workouts.empty())
        return;

    QList<int> workoutIds;
    workoutIds.reserve(static_cast<int>(workouts.size()));
    for (const auto& workout : workouts)
        workoutIds.append(workout.id());

    auto exercises = m_exerciseRepo.findByWorkoutIds(workoutIds);

    QList<int> exerciseIds;
    exerciseIds.reserve(static_cast<int>(exercises.size()));
    for (const auto& exercise : exercises)
        exerciseIds.append(exercise.id());

    auto sets = m_setRepo.findByExerciseIds(exerciseIds);

    QHash<int, std::vector<Set>> setsByExercise;
    for (auto& set : sets)
        setsByExercise[set.exerciseId()].push_back(std::move(set));

    QHash<int, std::vector<Exercise>> exercisesByWorkout;
    for (auto& exercise : exercises)
    {
        auto it = setsByExercise.find(exercise.id());
        if (it != setsByExercise.end())
        {
            for (auto& set : it.value())
                exercise.addSet(set);
        }
        exercisesByWorkout[exercise.workoutId()].push_back(std::move(exercise));
    }

    for (auto& workout : workouts)
    {
        auto it = exercisesByWorkout.find(workout.id());
        if (it == exercisesByWorkout.end())
            continue;
        for (const auto& exercise : it.value())
            workout.addExercise(exercise);
    }
}

void WorkoutRepositoryDb::saveChildren(int workoutId, const Workout& workout)
{
    QList<int> storedExerciseIds;
    for (const Exercise& stored : m_exerciseRepo.findByWorkoutId(workoutId))
        storedExerciseIds.append(stored.id());

    QHash<int, QList<int>> storedSetIds;
    for (const Set& stored : m_setRepo.findByExerciseIds(storedExerciseIds))
        storedSetIds[stored.exerciseId()].append(stored.id());

    QList<int> keptExerciseIds;
    for (const auto& exercise : workout.exercises())
    {
        Exercise e = exercise;
        e.setWorkoutId(workoutId);
        if (!storedExerciseIds.contains(e.id()))
            e.setId(-1);

        const int exerciseId = m_exerciseRepo.save(e);
        keptExerciseIds.append(exerciseId);

        const QList<int>& knownSetIds = storedSetIds[exerciseId];

        QList<int> keptSetIds;
        for (const auto& set : exercise.sets())
        {
            Set s = set;
            s.setExerciseId(exerciseId);
            if (!knownSetIds.contains(s.id()))
                s.setId(-1);

            keptSetIds.append(m_setRepo.save(s));
        }

        m_setRepo.removeByExerciseIdExcept(exerciseId, keptSetIds);
    }

    m_exerciseRepo.removeByWorkoutIdExcept(workoutId, keptExerciseIds);
}

Where WorkoutRepositoryDb::buildWhereClause(const WorkoutQuery& query) const
{
    Where where;

    if (query.id().has_value())
        addClause(where, Where(WorkoutSerializer::id_key).equals(query.id().value()));

    if (query.name().has_value())
        addClause(where, Where(WorkoutSerializer::name_key).equals(query.name().value()));

    if (query.createdAfter().has_value())
    {
        addClause(where,
                  Where(WorkoutSerializer::created_time_key)
                      .greaterThanOrEquals(query.createdAfter().value().toString(Qt::ISODate)));
    }

    if (query.createdBefore().has_value())
    {
        addClause(where,
                  Where(WorkoutSerializer::created_time_key)
                      .lessThanOrEquals(query.createdBefore().value().toString(Qt::ISODate)));
    }

    if (query.startedTimeIsNull().has_value())
    {
        addClause(
            where,
            isNullClause(WorkoutSerializer::started_time_key, query.startedTimeIsNull().value()));
    }

    if (query.plannedTimeIsNull().has_value())
    {
        addClause(
            where,
            isNullClause(WorkoutSerializer::planned_time_key, query.plannedTimeIsNull().value()));
    }

    if (query.status().has_value())
    {
        addClause(where,
                  Where(WorkoutSerializer::status_key)
                      .equals(workoutStatusToString(query.status().value())));
    }

    return where;
}

Order WorkoutRepositoryDb::buildOrderClause(const WorkoutQuery& query) const
{
    Order order;

    const auto appendOrder = [&](const char* column, SortDirection direction)
    {
        if (order.isEmpty())
            order = Order(column);
        else
            order.then(column);

        if (direction == SortDirection::Ascending)
            order.asc();
        else
            order.desc();
    };

    if (query.orderByCreatedTimeDirection().has_value())
        appendOrder(WorkoutSerializer::created_time_key,
                    query.orderByCreatedTimeDirection().value());

    if (query.orderByStartedTimeDirection().has_value())
        appendOrder(WorkoutSerializer::started_time_key,
                    query.orderByStartedTimeDirection().value());

    if (query.orderByPlannedTimeDirection().has_value())
        appendOrder(WorkoutSerializer::planned_time_key,
                    query.orderByPlannedTimeDirection().value());

    return order;
}
