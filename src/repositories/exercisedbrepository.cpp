#include "exercisedbrepository.h"

#include "../models/workoutmodel.h"
#include "workoutdbrepository.h"

ExerciseDbRepository::ExerciseDbRepository(QSqlDatabase &database, QObject *parent)
    : TemplateDbRepository(ExerciseDbRepository::exercises_table,
                           {ExerciseModel::id_key,
                            ExerciseModel::workout_id_key,
                            ExerciseModel::name_key,
                            ExerciseModel::description_key,
                            ExerciseModel::youtube_link_key,
                            ExerciseModel::rest_seconds_key},
                           database,
                           parent)
{
    QString queryStr = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
                                      "%2 INTEGER PRIMARY KEY AUTOINCREMENT, "
                                      "%3 INTEGER NOT NULL, "
                                      "%4 TEXT, "
                                      "%5 TEXT, "
                                      "%6 TEXT, "
                                      "%7 INTEGER, "
                                      "FOREIGN KEY(%3) REFERENCES %8(%9) ON DELETE CASCADE)")
                           .arg(ExerciseDbRepository::exercises_table,
                                ExerciseModel::id_key,
                                ExerciseModel::workout_id_key,
                                ExerciseModel::name_key,
                                ExerciseModel::description_key,
                                ExerciseModel::youtube_link_key,
                                ExerciseModel::rest_seconds_key,
                                WorkoutDbRepository::workouts_table,
                                WorkoutModel::id_key);

    createTable(queryStr);
}
