#include "exerciserepositorydb.h"
#include "infrastructure/dbrows.h"
#include "infrastructure/workout/exerciseserializer.h"
#include "infrastructure/workout/workoutserializer.h"

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

ExerciseRepositoryDb::ExerciseRepositoryDb(DbStorage& storage)
    : m_repository(std::make_unique<DbRepository>(
          ExerciseSerializer::table, ExerciseSerializer::id_key,
          QStringList { ExerciseSerializer::id_key, ExerciseSerializer::workout_id_key,
                        ExerciseSerializer::name_key, ExerciseSerializer::description_key,
                        ExerciseSerializer::rest_seconds_key, ExerciseSerializer::kind_key,
                        ExerciseSerializer::position_key, ExerciseSerializer::definition_id_key,
                        ExerciseSerializer::notes_key },
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
        .column(Column(ExerciseSerializer::rest_seconds_key).integer())
        .column(
            Column(ExerciseSerializer::kind_key).text().defaultValue(QStringLiteral("strength")))
        .column(Column(ExerciseSerializer::position_key).integer().defaultValue(0))
        .column(Column(ExerciseSerializer::definition_id_key).integer())
        .column(Column(ExerciseSerializer::notes_key).text())
        .foreignKey(ExerciseSerializer::workout_id_key, WorkoutSerializer::table,
                    WorkoutSerializer::id_key, OnDeleteAction::Cascade);
    return m_repository->createTable(table);
}

void ExerciseRepositoryDb::registerMigrations(MigrationRunner& runner)
{
    // v1: the youtube_link column was removed from the schema.
    runner.add(1,
               [](QSqlDatabase& db)
               {
                   return AlterTable(ExerciseSerializer::table)
                              .dropColumn(QStringLiteral("youtube_link"))
                              .execute(db)
                              .toInt()
                       != 0;
               });

    runner.add(3,
               [](QSqlDatabase& db)
               {
                   return AlterTable(ExerciseSerializer::table)
                              .addColumn(Column(ExerciseSerializer::kind_key)
                                             .text()
                                             .defaultValue(QStringLiteral("strength")))
                              .execute(db)
                              .toInt()
                       != 0;
               });

    runner.add(
        4,
        [](QSqlDatabase& db)
        {
            const bool columnsAdded
                = AlterTable(ExerciseSerializer::table)
                      .addColumn(Column(ExerciseSerializer::position_key).integer().defaultValue(0))
                      .addColumn(Column(ExerciseSerializer::definition_id_key).integer())
                      .addColumn(Column(ExerciseSerializer::notes_key).text())
                      .execute(db)
                      .toInt()
                != 0;

            return columnsAdded
                && backfillPositions(db, ExerciseSerializer::table,
                                     ExerciseSerializer::workout_id_key, ExerciseSerializer::id_key,
                                     ExerciseSerializer::position_key);
        });

    runner.add(6,
               [](QSqlDatabase& db)
               {
                   Where isStaticHold;
                   for (const QString& name : staticHoldNames())
                       isStaticHold.or_(Where(ExerciseSerializer::name_key).like(name));

                   return Update(ExerciseSerializer::table)
                              .set(ExerciseSerializer::kind_key,
                                   exerciseKindToString(ExerciseKind::Isometric))
                              .where(Where(ExerciseSerializer::kind_key)
                                         .equals(exerciseKindToString(ExerciseKind::Interval))
                                         .and_(isStaticHold))
                              .execute(db)
                              .toInt()
                       >= 0;
               });
}

QStringList ExerciseRepositoryDb::staticHoldNames()
{
    return { QStringLiteral("Plank"), QStringLiteral("Side Plank"), QStringLiteral("Hollow Hold"),
             QStringLiteral("Dead Hang"), QStringLiteral("Wall Sit") };
}

std::vector<Exercise> ExerciseRepositoryDb::findBy(const Where& where) const
{
    return DbRows::toEntities(m_repository->select(where,
                                                   positionOrder(ExerciseSerializer::workout_id_key,
                                                                 ExerciseSerializer::position_key)),
                              ExerciseSerializer::fromVariant);
}

std::vector<Exercise> ExerciseRepositoryDb::findByWorkoutId(int workoutId) const
{
    return findBy(Where(ExerciseSerializer::workout_id_key).equals(workoutId));
}

std::vector<Exercise> ExerciseRepositoryDb::findByWorkoutIds(const QList<int>& workoutIds) const
{
    if (workoutIds.isEmpty())
        return {};

    return findBy(Where(ExerciseSerializer::workout_id_key).in(workoutIds));
}

int ExerciseRepositoryDb::save(const Exercise& exercise)
{
    return m_repository->upsert(ExerciseSerializer::toVariant(exercise)).toInt();
}

void ExerciseRepositoryDb::removeByWorkoutIdExcept(int workoutId, const QList<int>& keptIds)
{
    auto where = Where(ExerciseSerializer::workout_id_key).equals(workoutId);
    if (!keptIds.isEmpty())
        where.and_(Where().not_(Where(ExerciseSerializer::id_key).in(keptIds)));

    m_repository->remove(where);
}
