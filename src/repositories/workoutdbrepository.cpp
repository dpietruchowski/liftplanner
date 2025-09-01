#include "workoutdbrepository.h"

WorkoutDbRepository::WorkoutDbRepository(QSqlDatabase &database, QObject *parent)
    : TemplateDbRepository(WorkoutDbRepository::workouts_table,
                           {WorkoutModel::id_key,
                            WorkoutModel::name_key,
                            WorkoutModel::created_time_key,
                            WorkoutModel::started_time_key,
                            WorkoutModel::ended_time_key},
                           database,
                           parent)
{
    QString queryStr = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
                                      "%2 INTEGER PRIMARY KEY AUTOINCREMENT, "
                                      "%3 TEXT, "
                                      "%4 TEXT, "
                                      "%5 TEXT, "
                                      "%6 TEXT)")
                           .arg(WorkoutDbRepository::workouts_table,
                                WorkoutModel::id_key,
                                WorkoutModel::name_key,
                                WorkoutModel::created_time_key,
                                WorkoutModel::started_time_key,
                                WorkoutModel::ended_time_key);

    createTable(queryStr);
}
