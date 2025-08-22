#include "workoutdbrepository.h"

WorkoutDbRepository::WorkoutDbRepository(QSqlDatabase &database, QObject *parent)
    : TemplateDbRepository(WorkoutDbRepository::workouts_table,
                           {WorkoutModel::id_key, WorkoutModel::name_key},
                           database,
                           parent)
{
    QString queryStr = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
                                      "%2 INTEGER PRIMARY KEY AUTOINCREMENT, "
                                      "%3 TEXT)")
                           .arg(WorkoutDbRepository::workouts_table,
                                WorkoutModel::id_key,
                                WorkoutModel::name_key);

    createTable(queryStr);
}
