#include "exercisedbrepository.h"

#include "../models/workoutmodel.h"
#include "workoutdbrepository.h"

ExerciseDbRepository::ExerciseDbRepository(QSqlDatabase &database, QObject *parent)
    : TemplateDbRepository(ExerciseDbRepository::exercises_table,
                           {ExerciseModel::id_key,
                            ExerciseModel::workout_id_key,
                            ExerciseModel::name_key},
                           database,
                           parent)
{
    QString queryStr = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
                                      "%2 INTEGER PRIMARY KEY AUTOINCREMENT, "
                                      "%3 INTEGER NOT NULL, "
                                      "%4 TEXT, "
                                      "FOREIGN KEY(%3) REFERENCES %5(%6) ON DELETE CASCADE)")
                           .arg(ExerciseDbRepository::exercises_table,
                                ExerciseModel::id_key,
                                ExerciseModel::workout_id_key,
                                ExerciseModel::name_key,
                                WorkoutDbRepository::workouts_table,
                                WorkoutModel::id_key);

    createTable(queryStr);
}
