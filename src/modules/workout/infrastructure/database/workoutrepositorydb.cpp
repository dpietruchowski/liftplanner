#include "workoutrepositorydb.h"
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/entities/set.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/infrastructure/serializers/workoutserializer.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/where.h>
#include <dbtoolkit/query/order.h>

WorkoutRepositoryDb::WorkoutRepositoryDb(DbStorage &storage)
    : m_workoutRepo(std::make_unique<DbRepository>(
          WorkoutSerializer::table,
          WorkoutSerializer::id_key,
          QStringList{
              WorkoutSerializer::id_key,
              WorkoutSerializer::name_key,
              WorkoutSerializer::created_time_key,
              WorkoutSerializer::started_time_key,
              WorkoutSerializer::ended_time_key},
          storage, nullptr)),
      m_exerciseRepo(storage),
      m_setRepo(storage)
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
        .column(Column(WorkoutSerializer::started_time_key).text())
        .column(Column(WorkoutSerializer::ended_time_key).text());

    return m_workoutRepo->createTable(workouts) && m_exerciseRepo.createTable() && m_setRepo.createTable();
}

std::vector<Workout> WorkoutRepositoryDb::findAll(const WorkoutQuery &query) const
{
    auto where = buildWhereClause(query);
    auto order = buildOrderClause(query);
    int limit = query.limit().value_or(-1);
    int offset = query.offset().value_or(-1);

    auto rows = m_workoutRepo->select(where, order, limit, offset);

    std::vector<Workout> results;
    results.reserve(rows.size());
    for (const auto &row : rows)
    {
        Workout w = WorkoutSerializer::fromVariant(row);
        loadChildren(w);
        results.push_back(std::move(w));
    }
    return results;
}

std::optional<Workout> WorkoutRepositoryDb::findOne(const WorkoutQuery &query) const
{
    auto where = buildWhereClause(query);
    auto rows = m_workoutRepo->select(where, Order(), 1);
    if (rows.isEmpty())
        return std::nullopt;

    Workout w = WorkoutSerializer::fromVariant(rows.first());
    loadChildren(w);
    return w;
}

int WorkoutRepositoryDb::save(const Workout &workout)
{
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
    return workoutId;
}

bool WorkoutRepositoryDb::remove(const WorkoutQuery &query)
{
    auto where = buildWhereClause(query);
    return m_workoutRepo->remove(where) > 0;
}

int WorkoutRepositoryDb::count(const WorkoutQuery &query) const
{
    auto where = buildWhereClause(query);
    return m_workoutRepo->count(where);
}

bool WorkoutRepositoryDb::exists(const WorkoutQuery &query) const
{
    auto where = buildWhereClause(query);
    return m_workoutRepo->exists(where);
}

void WorkoutRepositoryDb::loadChildren(Workout &workout) const
{
    auto exercises = m_exerciseRepo.findByWorkoutId(workout.id());

    for (auto &exercise : exercises)
    {
        auto sets = m_setRepo.findByExerciseId(exercise.id());
        for (auto &set : sets)
            exercise.addSet(set);

        workout.addExercise(exercise);
    }
}

void WorkoutRepositoryDb::saveChildren(int workoutId, const Workout &workout)
{
    for (const auto &exercise : workout.exercises())
    {
        Exercise e = exercise;
        e.setWorkoutId(workoutId);

        int exerciseId = m_exerciseRepo.save(e);

        for (const auto &set : exercise.sets())
        {
            Set s = set;
            s.setExerciseId(exerciseId);
            m_setRepo.save(s);
        }
    }
}

Where WorkoutRepositoryDb::buildWhereClause(const WorkoutQuery &query) const
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
        auto afterWhere = Where(WorkoutSerializer::created_time_key)
                              .greaterThanOrEquals(query.createdAfter().value().toString(Qt::ISODate));
        where = where.isEmpty() ? afterWhere : where.and_(afterWhere);
    }

    if (query.createdBefore().has_value())
    {
        auto beforeWhere = Where(WorkoutSerializer::created_time_key)
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

    return where;
}

Order WorkoutRepositoryDb::buildOrderClause(const WorkoutQuery &query) const
{
    Order order;

    if (query.orderByCreatedTimeDirection().has_value())
    {
        order = Order(WorkoutSerializer::created_time_key);
        if (query.orderByCreatedTimeDirection().value() == SortDirection::Ascending)
            order.asc();
        else
            order.desc();
    }

    if (query.orderByStartedTimeDirection().has_value())
    {
        auto stOrder = Order(WorkoutSerializer::started_time_key);
        if (query.orderByStartedTimeDirection().value() == SortDirection::Ascending)
            stOrder.asc();
        else
            stOrder.desc();
        order = order.isEmpty() ? stOrder : order.then(WorkoutSerializer::started_time_key);
        if (query.orderByStartedTimeDirection().value() == SortDirection::Ascending)
            order.asc();
        else
            order.desc();
    }

    return order;
}
