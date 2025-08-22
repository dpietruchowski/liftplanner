#pragma once
#include "templatedbrepository.h"
#include "../models/workoutmodel.h"

class WorkoutDbRepository : public TemplateDbRepository<WorkoutModel>
{
public:
    DEFINE_TABLE_NAME(workouts);
    explicit WorkoutDbRepository(QSqlDatabase &database, QObject *parent = nullptr);
};
