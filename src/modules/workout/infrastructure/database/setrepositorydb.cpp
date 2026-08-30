#include "setrepositorydb.h"
#include "modules/workout/infrastructure/serializers/exerciseserializer.h"
#include "modules/workout/infrastructure/serializers/setserializer.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>
#include <dbtoolkit/query/altertable.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/select.h>
#include <dbtoolkit/query/update.h>
#include <dbtoolkit/query/where.h>

#include "positionbackfill.h"

#include <QSqlQuery>

SetRepositoryDb::SetRepositoryDb(DbStorage& storage)
    : m_repository(std::make_unique<DbRepository>(
          SetSerializer::table, SetSerializer::id_key,
          QStringList { SetSerializer::id_key, SetSerializer::exercise_id_key,
                        SetSerializer::repetitions_key, SetSerializer::weight_key,
                        SetSerializer::completed_key, SetSerializer::metric_key,
                        SetSerializer::load_type_key, SetSerializer::duration_seconds_key,
                        SetSerializer::distance_meters_key,
                        SetSerializer::rest_seconds_override_key, SetSerializer::position_key },
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
        .column(Column(SetSerializer::weight_key).real())
        .column(Column(SetSerializer::completed_key).integer().defaultValue(0))
        .column(Column(SetSerializer::metric_key).text().defaultValue(QStringLiteral("reps")))
        .column(
            Column(SetSerializer::load_type_key).text().defaultValue(QStringLiteral("external")))
        .column(Column(SetSerializer::duration_seconds_key).integer().defaultValue(0))
        .column(Column(SetSerializer::distance_meters_key).real().defaultValue(0.0))
        .column(Column(SetSerializer::rest_seconds_override_key).integer().defaultValue(-1))
        .column(Column(SetSerializer::position_key).integer().defaultValue(0))
        .foreignKey(SetSerializer::exercise_id_key, ExerciseSerializer::table,
                    ExerciseSerializer::id_key, OnDeleteAction::Cascade);
    bool ok = m_repository->createTable(table);

    QSqlQuery pragmaQuery(m_repository->storage().database());
    pragmaQuery.exec(QStringLiteral("PRAGMA table_info(%1)").arg(SetSerializer::table));
    bool hasCompletedColumn = false;
    while (pragmaQuery.next())
    {
        if (pragmaQuery.value(1).toString() == SetSerializer::completed_key)
        {
            hasCompletedColumn = true;
            break;
        }
    }
    if (!hasCompletedColumn)
    {
        QSqlQuery alter(m_repository->storage().database());
        alter.exec(QStringLiteral("ALTER TABLE %1 ADD COLUMN %2 INTEGER DEFAULT 0")
                       .arg(SetSerializer::table, SetSerializer::completed_key));
    }

    return ok;
}

void SetRepositoryDb::registerMigrations(MigrationRunner& runner)
{
    runner.add(
        2,
        [](QSqlDatabase& db)
        {
            return AlterTable(SetSerializer::table)
                       .addColumn(Column(SetSerializer::metric_key)
                                      .text()
                                      .defaultValue(QStringLiteral("reps")))
                       .addColumn(Column(SetSerializer::load_type_key)
                                      .text()
                                      .defaultValue(QStringLiteral("external")))
                       .addColumn(
                           Column(SetSerializer::duration_seconds_key).integer().defaultValue(0))
                       .addColumn(
                           Column(SetSerializer::distance_meters_key).real().defaultValue(0.0))
                       .addColumn(Column(SetSerializer::rest_seconds_override_key)
                                      .integer()
                                      .defaultValue(-1))
                       .execute(db)
                       .toInt()
                != 0;
        });

    runner.add(
        5,
        [](QSqlDatabase& db)
        {
            const bool columnAdded
                = AlterTable(SetSerializer::table)
                      .addColumn(Column(SetSerializer::position_key).integer().defaultValue(0))
                      .execute(db)
                      .toInt()
                != 0;

            return columnAdded
                && backfillPositions(db, SetSerializer::table, SetSerializer::exercise_id_key,
                                     SetSerializer::id_key, SetSerializer::position_key);
        });
}

std::vector<Set> SetRepositoryDb::findByExerciseId(int exerciseId) const
{
    auto where = Where(SetSerializer::exercise_id_key).equals(exerciseId);
    auto rows = m_repository->select(
        where, positionOrder(SetSerializer::exercise_id_key, SetSerializer::position_key));

    std::vector<Set> results;
    results.reserve(rows.size());
    for (const auto& row : rows)
        results.push_back(SetSerializer::fromVariant(row));
    return results;
}

std::vector<Set> SetRepositoryDb::findByExerciseIds(const QList<int>& exerciseIds) const
{
    if (exerciseIds.isEmpty())
        return {};

    auto where = Where(SetSerializer::exercise_id_key).in(exerciseIds);
    auto rows = m_repository->select(
        where, positionOrder(SetSerializer::exercise_id_key, SetSerializer::position_key));

    std::vector<Set> results;
    results.reserve(rows.size());
    for (const auto& row : rows)
        results.push_back(SetSerializer::fromVariant(row));
    return results;
}

int SetRepositoryDb::save(const Set& set)
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

    return m_repository->insert(data).toInt();
}
