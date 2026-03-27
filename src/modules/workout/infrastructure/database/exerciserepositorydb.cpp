#include "exerciserepositorydb.h"
#include "modules/workout/domain/entities/exercise.h"
#include "modules/workout/domain/repositories/exercisequery.h"
#include "modules/workout/infrastructure/serializers/exerciseserializer.h"
#include "modules/workout/infrastructure/serializers/workoutserializer.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/where.h>
#include <dbtoolkit/query/order.h>

ExerciseRepositoryDb::ExerciseRepositoryDb(DbStorage &storage)
    : m_repository(std::make_unique<DbRepository>(
          ExerciseSerializer::table,
          ExerciseSerializer::id_key,
          QStringList{
              ExerciseSerializer::id_key,
              ExerciseSerializer::workout_id_key,
              ExerciseSerializer::name_key,
              ExerciseSerializer::description_key,
              ExerciseSerializer::youtube_link_key,
              ExerciseSerializer::rest_seconds_key},
          storage, nullptr))
{
}

ExerciseRepositoryDb::~ExerciseRepositoryDb() = default;

bool ExerciseRepositoryDb::createTable()
{
    CreateTable table(ExerciseSerializer::table);
    table.ifNotExists()
        .column(Column(ExerciseSerializer::id_key).integer().primaryKey().autoIncrement().notNull())
        .column(Column(ExerciseSerializer::workout_id_key).integer().notNull())
        .column(Column(ExerciseSerializer::name_key).text())
        .column(Column(ExerciseSerializer::description_key).text())
        .column(Column(ExerciseSerializer::youtube_link_key).text())
        .column(Column(ExerciseSerializer::rest_seconds_key).integer())
        .foreignKey(ExerciseSerializer::workout_id_key,
                    WorkoutSerializer::table,
                    WorkoutSerializer::id_key,
                    OnDeleteAction::Cascade);
    return m_repository->createTable(table);
}

std::vector<Exercise> ExerciseRepositoryDb::findAll(const ExerciseQuery &query) const
{
    auto where = buildWhereClause(query);
    int limit = query.limit().value_or(-1);

    auto rows = m_repository->select(where, Order(), limit);

    std::vector<Exercise> results;
    results.reserve(rows.size());
    for (const auto &row : rows)
    {
        results.push_back(ExerciseSerializer::fromVariant(row));
    }
    return results;
}

std::optional<Exercise> ExerciseRepositoryDb::findOne(const ExerciseQuery &query) const
{
    auto where = buildWhereClause(query);
    auto rows = m_repository->select(where, Order(), 1);
    if (rows.isEmpty())
        return std::nullopt;
    return ExerciseSerializer::fromVariant(rows.first());
}

int ExerciseRepositoryDb::save(const Exercise &exercise)
{
    QVariantMap data = ExerciseSerializer::toVariant(exercise);

    if (exercise.id() != -1)
    {
        auto where = Where(ExerciseSerializer::id_key).equals(exercise.id());
        if (m_repository->exists(where))
        {
            m_repository->update(data, where);
            return exercise.id();
        }
    }

    auto result = m_repository->insert(data);
    return result.toInt();
}

bool ExerciseRepositoryDb::remove(const ExerciseQuery &query)
{
    auto where = buildWhereClause(query);
    return m_repository->remove(where) > 0;
}

int ExerciseRepositoryDb::count(const ExerciseQuery &query) const
{
    auto where = buildWhereClause(query);
    return m_repository->count(where);
}

bool ExerciseRepositoryDb::exists(const ExerciseQuery &query) const
{
    auto where = buildWhereClause(query);
    return m_repository->exists(where);
}

Where ExerciseRepositoryDb::buildWhereClause(const ExerciseQuery &query) const
{
    Where where;

    if (query.id().has_value())
        where = Where(ExerciseSerializer::id_key).equals(query.id().value());

    if (query.workoutId().has_value())
    {
        auto wkWhere = Where(ExerciseSerializer::workout_id_key).equals(query.workoutId().value());
        where = where.isEmpty() ? wkWhere : where.and_(wkWhere);
    }

    if (query.name().has_value())
    {
        auto nameWhere = Where(ExerciseSerializer::name_key).equals(query.name().value());
        where = where.isEmpty() ? nameWhere : where.and_(nameWhere);
    }

    return where;
}
