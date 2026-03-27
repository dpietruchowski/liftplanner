#include "exerciserepositorydb.h"
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

std::vector<Exercise> ExerciseRepositoryDb::findByWorkoutId(int workoutId) const
{
    auto where = Where(ExerciseSerializer::workout_id_key).equals(workoutId);
    auto rows = m_repository->select(where);

    std::vector<Exercise> results;
    results.reserve(rows.size());
    for (const auto &row : rows)
        results.push_back(ExerciseSerializer::fromVariant(row));
    return results;
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

    return m_repository->insert(data).toInt();
}

void ExerciseRepositoryDb::removeByWorkoutId(int workoutId)
{
    auto where = Where(ExerciseSerializer::workout_id_key).equals(workoutId);
    m_repository->remove(where);
}
