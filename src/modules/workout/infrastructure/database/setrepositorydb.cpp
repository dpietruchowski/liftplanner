#include "setrepositorydb.h"
#include "modules/workout/domain/entities/set.h"
#include "modules/workout/domain/repositories/setquery.h"
#include "modules/workout/infrastructure/serializers/setserializer.h"
#include "modules/workout/infrastructure/serializers/exerciseserializer.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/where.h>
#include <dbtoolkit/query/order.h>

SetRepositoryDb::SetRepositoryDb(DbStorage &storage)
    : m_repository(std::make_unique<DbRepository>(
          SetSerializer::table,
          SetSerializer::id_key,
          QStringList{
              SetSerializer::id_key,
              SetSerializer::exercise_id_key,
              SetSerializer::repetitions_key,
              SetSerializer::weight_key},
          storage, nullptr))
{
}

SetRepositoryDb::~SetRepositoryDb() = default;

bool SetRepositoryDb::createTable()
{
    CreateTable table(SetSerializer::table);
    table.ifNotExists()
        .column(Column(SetSerializer::id_key).integer().primaryKey().autoIncrement().notNull())
        .column(Column(SetSerializer::exercise_id_key).integer().notNull())
        .column(Column(SetSerializer::repetitions_key).integer())
        .column(Column(SetSerializer::weight_key).integer())
        .foreignKey(SetSerializer::exercise_id_key,
                    ExerciseSerializer::table,
                    ExerciseSerializer::id_key,
                    OnDeleteAction::Cascade);
    return m_repository->createTable(table);
}

std::vector<Set> SetRepositoryDb::findAll(const SetQuery &query) const
{
    auto where = buildWhereClause(query);
    int limit = query.limit().value_or(-1);

    auto rows = m_repository->select(where, Order(), limit);

    std::vector<Set> results;
    results.reserve(rows.size());
    for (const auto &row : rows)
    {
        results.push_back(SetSerializer::fromVariant(row));
    }
    return results;
}

std::optional<Set> SetRepositoryDb::findOne(const SetQuery &query) const
{
    auto where = buildWhereClause(query);
    auto rows = m_repository->select(where, Order(), 1);
    if (rows.isEmpty())
        return std::nullopt;
    return SetSerializer::fromVariant(rows.first());
}

int SetRepositoryDb::save(const Set &set)
{
    QVariantMap data = SetSerializer::toVariant(set);

    if (set.id() != -1)
    {
        auto where = Where(SetSerializer::id_key).equals(set.id());
        if (m_repository->exists(where))
        {
            m_repository->update(data, where);
            return set.id();
        }
    }

    auto result = m_repository->insert(data);
    return result.toInt();
}

bool SetRepositoryDb::remove(const SetQuery &query)
{
    auto where = buildWhereClause(query);
    return m_repository->remove(where) > 0;
}

int SetRepositoryDb::count(const SetQuery &query) const
{
    auto where = buildWhereClause(query);
    return m_repository->count(where);
}

bool SetRepositoryDb::exists(const SetQuery &query) const
{
    auto where = buildWhereClause(query);
    return m_repository->exists(where);
}

Where SetRepositoryDb::buildWhereClause(const SetQuery &query) const
{
    Where where;

    if (query.id().has_value())
        where = Where(SetSerializer::id_key).equals(query.id().value());

    if (query.exerciseId().has_value())
    {
        auto exWhere = Where(SetSerializer::exercise_id_key).equals(query.exerciseId().value());
        where = where.isEmpty() ? exWhere : where.and_(exWhere);
    }

    return where;
}
