#include "workoutrepositorydb.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/workoutstatus.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/infrastructure/serializers/workoutserializer.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/where.h>

#include <QHash>
#include <QSqlQuery>

WorkoutRepositoryDb::WorkoutRepositoryDb(DbStorage& storage)
    : m_workoutRepo(std::make_unique<DbRepository>(
          WorkoutSerializer::table, WorkoutSerializer::id_key,
          QStringList { WorkoutSerializer::id_key, WorkoutSerializer::name_key,
                        WorkoutSerializer::created_time_key, WorkoutSerializer::planned_time_key,
                        WorkoutSerializer::started_time_key, WorkoutSerializer::ended_time_key,
                        WorkoutSerializer::status_key },
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
        .column(Column(WorkoutSerializer::status_key).text());

    bool ok = m_workoutRepo->createTable(workouts) && m_exerciseRepo.createTable()
        && m_setRepo.createTable();

    // Migration: add status column if missing (existing databases)
    QSqlQuery pragmaQuery(m_workoutRepo->storage().database());
    pragmaQuery.exec(QStringLiteral("PRAGMA table_info(%1)").arg(WorkoutSerializer::table));
    bool hasStatusColumn = false;
    while (pragmaQuery.next())
    {
        if (pragmaQuery.value(1).toString() == WorkoutSerializer::status_key)
        {
            hasStatusColumn = true;
            break;
        }
    }
    if (!hasStatusColumn)
    {
        QSqlQuery alter(m_workoutRepo->storage().database());
        alter.exec(QStringLiteral("ALTER TABLE %1 ADD COLUMN %2 TEXT DEFAULT 'Planned'")
                       .arg(WorkoutSerializer::table, WorkoutSerializer::status_key));
        alter.exec(QStringLiteral("UPDATE %1 SET %2 = 'Ended' WHERE %3 IS NOT NULL")
                       .arg(WorkoutSerializer::table, WorkoutSerializer::status_key,
                            WorkoutSerializer::ended_time_key));
        alter.exec(
            QStringLiteral("UPDATE %1 SET %2 = 'Started' WHERE %3 IS NOT NULL AND %4 IS NULL")
                .arg(WorkoutSerializer::table, WorkoutSerializer::status_key,
                     WorkoutSerializer::started_time_key, WorkoutSerializer::ended_time_key));
    }

    return ok;
}

void WorkoutRepositoryDb::registerMigrations(MigrationRunner& runner)
{
    m_exerciseRepo.registerMigrations(runner);
}

std::vector<Workout> WorkoutRepositoryDb::findAll(const WorkoutQuery& query) const
{
    auto where = buildWhereClause(query);
    auto order = buildOrderClause(query);
    int limit = query.limit().value_or(-1);
    int offset = query.offset().value_or(-1);

    auto rows = m_workoutRepo->select(where, order, limit, offset);

    std::vector<Workout> results;
    results.reserve(rows.size());
    for (const auto& row : rows)
        results.push_back(WorkoutSerializer::fromVariant(row));

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

    QVariantMap data = WorkoutSerializer::toVariant(workout);

    int workoutId;
    if (workout.id() != -1)
    {
        auto where = Where(WorkoutSerializer::id_key).equals(workout.id());
        if (m_workoutRepo->exists(where))
        {
            m_workoutRepo->update(data, where);
            workoutId = workout.id();
        }
        else
        {
            workoutId = m_workoutRepo->insert(data).toInt();
        }
    }
    else
    {
        workoutId = m_workoutRepo->insert(data).toInt();
    }

    saveChildren(workoutId, workout);

    storage.commit();
    return workoutId;
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
    m_exerciseRepo.removeByWorkoutId(workoutId);

    for (const auto& exercise : workout.exercises())
    {
        Exercise e = exercise;
        e.setWorkoutId(workoutId);

        int exerciseId = m_exerciseRepo.save(e);

        for (const auto& set : exercise.sets())
        {
            Set s = set;
            s.setExerciseId(exerciseId);
            m_setRepo.save(s);
        }
    }
}

Where WorkoutRepositoryDb::buildWhereClause(const WorkoutQuery& query) const
{
    Where where;

    if (query.id().has_value())
        where = Where(WorkoutSerializer::id_key).equals(query.id().value());

    if (query.name().has_value())
    {
        auto nameWhere = Where(WorkoutSerializer::name_key).equals(query.name().value());
        where = where.isEmpty() ? nameWhere : where.and_(nameWhere);
    }

    if (query.createdAfter().has_value())
    {
        auto afterWhere
            = Where(WorkoutSerializer::created_time_key)
                  .greaterThanOrEquals(query.createdAfter().value().toString(Qt::ISODate));
        where = where.isEmpty() ? afterWhere : where.and_(afterWhere);
    }

    if (query.createdBefore().has_value())
    {
        auto beforeWhere
            = Where(WorkoutSerializer::created_time_key)
                  .lessThanOrEquals(query.createdBefore().value().toString(Qt::ISODate));
        where = where.isEmpty() ? beforeWhere : where.and_(beforeWhere);
    }

    if (query.startedTimeIsNull().has_value())
    {
        Where stWhere;
        if (query.startedTimeIsNull().value())
            stWhere = Where(WorkoutSerializer::started_time_key).isNull();
        else
            stWhere = Where(WorkoutSerializer::started_time_key).isNotNull();
        where = where.isEmpty() ? stWhere : where.and_(stWhere);
    }

    if (query.plannedTimeIsNull().has_value())
    {
        Where ptWhere;
        if (query.plannedTimeIsNull().value())
            ptWhere = Where(WorkoutSerializer::planned_time_key).isNull();
        else
            ptWhere = Where(WorkoutSerializer::planned_time_key).isNotNull();
        where = where.isEmpty() ? ptWhere : where.and_(ptWhere);
    }

    if (query.status().has_value())
    {
        auto statusWhere = Where(WorkoutSerializer::status_key)
                               .equals(workoutStatusToString(query.status().value()));
        where = where.isEmpty() ? statusWhere : where.and_(statusWhere);
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
        appendOrder(WorkoutSerializer::created_time_key, query.orderByCreatedTimeDirection().value());

    if (query.orderByStartedTimeDirection().has_value())
        appendOrder(WorkoutSerializer::started_time_key, query.orderByStartedTimeDirection().value());

    if (query.orderByPlannedTimeDirection().has_value())
        appendOrder(WorkoutSerializer::planned_time_key, query.orderByPlannedTimeDirection().value());

    return order;
}
