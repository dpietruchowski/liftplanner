#include "setdbrepository.h"

#include "../models/exercisemodel.h"
#include "exercisedbrepository.h"

SetDbRepository::SetDbRepository(QSqlDatabase &database, QObject *parent)
    : TemplateDbRepository(SetDbRepository::sets_table,
                           {SetModel::id_key,
                            SetModel::exercise_id_key,
                            SetModel::repetitions_key,
                            SetModel::weight_key},
                           database,
                           parent)
{
    QString queryStr = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
                                      "%2 INTEGER PRIMARY KEY AUTOINCREMENT, "
                                      "%3 INTEGER NOT NULL, "
                                      "%4 INTEGER, "
                                      "%5 INTEGER, "
                                      "FOREIGN KEY(%3) REFERENCES %6(%7) ON DELETE CASCADE)")
                           .arg(SetDbRepository::sets_table,
                                SetModel::id_key,
                                SetModel::exercise_id_key,
                                SetModel::repetitions_key,
                                SetModel::weight_key,
                                ExerciseDbRepository::exercises_table,
                                ExerciseModel::id_key);

    createTable(queryStr);
}
