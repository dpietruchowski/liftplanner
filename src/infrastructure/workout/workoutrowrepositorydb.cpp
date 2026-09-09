#include "workoutrowrepositorydb.h"

#include "domain/workout/workoutstatus.h"
#include "infrastructure/workout/exerciseserializer.h"
#include "infrastructure/workout/setserializer.h"
#include "infrastructure/workout/workoutserializer.h"

#include <dbtoolkit/dbprojection.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/query/alias.h>
#include <dbtoolkit/query/join.h>
#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/projection.h>
#include <dbtoolkit/query/select.h>
#include <dbtoolkit/query/where.h>

namespace
{

const TableAlias workouts("w");
const TableAlias exercises("e");
const TableAlias sets("s");

HistorySetRow toRow(const ProjectedRow& row)
{
    const QVariantMap workoutGroup = row.of(workouts.prefix());
    const QVariantMap exerciseGroup = row.of(exercises.prefix());
    const QVariantMap setGroup = row.of(sets.prefix());

    HistorySetRow result;
    result.workoutId = workoutGroup.value(WorkoutSerializer::id_key).toInt();
    result.exerciseId = exerciseGroup.value(ExerciseSerializer::id_key).toInt();
    result.exerciseName = exerciseGroup.value(ExerciseSerializer::name_key).toString();
    result.hasSet = !setGroup.value(SetSerializer::id_key).isNull();
    result.completed = setGroup.value(SetSerializer::completed_key).toInt() != 0;
    result.metric = setMetricFromString(setGroup.value(SetSerializer::metric_key).toString());
    result.loadType = loadTypeFromString(setGroup.value(SetSerializer::load_type_key).toString());
    result.repetitions = setGroup.value(SetSerializer::repetitions_key).toInt();
    result.weight = setGroup.value(SetSerializer::weight_key).toDouble();
    result.durationSeconds = setGroup.value(SetSerializer::duration_seconds_key).toInt();
    result.distanceMeters = setGroup.value(SetSerializer::distance_meters_key).toDouble();
    return result;
}

QList<Projection> projections()
{
    return { Projection(workouts, { WorkoutSerializer::id_key }),
             Projection(exercises, { ExerciseSerializer::id_key, ExerciseSerializer::name_key }),
             Projection(sets,
                        { SetSerializer::id_key, SetSerializer::completed_key,
                          SetSerializer::metric_key, SetSerializer::load_type_key,
                          SetSerializer::repetitions_key, SetSerializer::weight_key,
                          SetSerializer::duration_seconds_key,
                          SetSerializer::distance_meters_key }) };
}

Select recentWorkoutIds(int workoutLimit)
{
    Select selected(QStringList { WorkoutSerializer::id_key });
    selected.from(WorkoutSerializer::table)
        .where(Where(WorkoutSerializer::status_key)
                   .equals(workoutStatusToString(WorkoutStatus::Ended)))
        .orderBy(Order(WorkoutSerializer::started_time_key).desc());

    if (workoutLimit > 0)
        selected.limit(workoutLimit);

    return selected;
}

Select shape(int workoutLimit)
{
    Select select;
    select.from(WorkoutSerializer::table)
        .as(workouts)
        .leftJoin(Join(ExerciseSerializer::table)
                      .as(exercises)
                      .on(workouts, WorkoutSerializer::id_key)
                      .equals(ExerciseSerializer::workout_id_key))
        .leftJoin(Join(SetSerializer::table)
                      .as(sets)
                      .on(exercises, ExerciseSerializer::id_key)
                      .equals(SetSerializer::exercise_id_key))
        .where(Where(workouts.createColumn(WorkoutSerializer::id_key))
                   .in(recentWorkoutIds(workoutLimit)))
        .orderBy(Order(workouts.createColumn(WorkoutSerializer::started_time_key))
                     .desc()
                     .then(exercises.createColumn(ExerciseSerializer::position_key))
                     .asc()
                     .then(sets.createColumn(SetSerializer::position_key))
                     .asc());

    return select;
}

}

WorkoutRowRepositoryDb::WorkoutRowRepositoryDb(DbStorage& storage)
    : m_storage(storage)
{
}

std::vector<HistorySetRow> WorkoutRowRepositoryDb::findSetsOfRecentWorkouts(int workoutLimit) const
{
    const DbProjection<HistorySetRow> projection(m_storage, projections(), &toRow);
    return projection.findAll(shape(workoutLimit));
}
