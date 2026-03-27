#include "workoutrepositorydb.h"
#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/repositories/workoutquery.h"
#include "modules/workout/infrastructure/serializers/workoutserializer.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/where.h>
#include <dbtoolkit/query/order.h>

WorkoutRepositoryDb::WorkoutRepositoryDb(DbStorage &storage)
    : m_repository(std::make_unique<DbRepository>(
          WorkoutSerializer::table,
          WorkoutSerializer::id_key,
          QStringList{
              WorkoutSerializer::id_key,
              WorkoutSerializer::name_key,
              WorkoutSerializer::created_time_key,
              WorkoutSerializer::started_time_key,
              WorkoutSerializer::ended_time_key},
          storage, nullptr))
{
}

WorkoutRepositoryDb::~WorkoutRepositoryDb() = default;

bool WorkoutRepositoryDb::createTable()
{
    CreateTable table(WorkoutSerializer::table);
    table.ifNotExists()
        .column(Column(WorkoutSerializer::id_key).integer().primaryKey().autoIncrement().notNull())
        .column(Column(WorkoutSerializer::name_key).text())
        .column(Column(WorkoutSerializer::created_time_key).text())
        .column(Column(WorkoutSerializer::started_time_key).text())
        .column(Column(WorkoutSerializer::ended_time_key).text());
    return m_repository->createTable(table);
}

std::vector<Workout> WorkoutRepositoryDb::findAll(const WorkoutQuery &query) const
{
    auto where = buildWhereClause(query);
    auto order = buildOrderClause(query);
    int limit = query.limit().value_or(-1);
    int offset = query.offset().value_or(-1);

    auto rows = m_repository->select(where, order, limit, offset);

    std::vector<Workout> results;
    results.reserve(rows.size());
    for (const auto &row : rows)
    {
        results.push_back(WorkoutSerializer::fromVariant(row));
    }
    return results;
}

std::optional<Workout> WorkoutRepositoryDb::findOne(const WorkoutQuery &query) const
{
    auto where = buildWhereClause(query);
    auto rows = m_repository->select(where, Order(), 1);
    if (rows.isEmpty())
        return std::nullopt;
    return WorkoutSerializer::fromVariant(rows.first());
}

int WorkoutRepositoryDb::save(const Workout &workout)
{
    QVariantMap data = WorkoutSerializer::toVariant(workout);

    if (workout.id() != -1)
    {
        auto where = Where(WorkoutSerializer::id_key).equals(workout.id());
        if (m_repository->exists(where))
        {
            m_repository->update(data, where);
            return workout.id();
        }
    }

    auto result = m_repository->insert(data);
    return result.toInt();
}

bool WorkoutRepositoryDb::remove(const WorkoutQuery &query)
{
    auto where = buildWhereClause(query);
    return m_repository->remove(where) > 0;
}

int WorkoutRepositoryDb::count(const WorkoutQuery &query) const
{
    auto where = buildWhereClause(query);
    return m_repository->count(where);
}

bool WorkoutRepositoryDb::exists(const WorkoutQuery &query) const
{
    auto where = buildWhereClause(query);
    return m_repository->exists(where);
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
