#include "setrepositorydb.h"
#include "infrastructure/dbrows.h"
#include "infrastructure/workout/exerciseserializer.h"
#include "infrastructure/workout/setserializer.h"

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
    return m_repository->createTable(table);
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

    runner.add(7,
               [](QSqlDatabase& db)
               {
                   return AlterTable(SetSerializer::table)
                              .addColumn(
                                  Column(SetSerializer::completed_key).integer().defaultValue(0))
                              .execute(db)
                              .toInt()
                       != 0;
               });
}

std::vector<Set> SetRepositoryDb::findBy(const Where& where) const
{
    return DbRows::toEntities(
        m_repository->select(
            where, positionOrder(SetSerializer::exercise_id_key, SetSerializer::position_key)),
        SetSerializer::fromVariant);
}

std::vector<Set> SetRepositoryDb::findByExerciseId(int exerciseId) const
{
    return findBy(Where(SetSerializer::exercise_id_key).equals(exerciseId));
}

std::vector<Set> SetRepositoryDb::findByExerciseIds(const QList<int>& exerciseIds) const
{
    if (exerciseIds.isEmpty())
        return {};

    return findBy(Where(SetSerializer::exercise_id_key).in(exerciseIds));
}

int SetRepositoryDb::save(const Set& set)
{
    return m_repository->upsert(SetSerializer::toVariant(set)).toInt();
}

void SetRepositoryDb::removeByExerciseIdExcept(int exerciseId, const QList<int>& keptIds)
{
    auto where = Where(SetSerializer::exercise_id_key).equals(exerciseId);
    if (!keptIds.isEmpty())
        where.and_(Where().not_(Where(SetSerializer::id_key).in(keptIds)));

    m_repository->remove(where);
}
