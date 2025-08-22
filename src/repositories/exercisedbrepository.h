#pragma once
#include "templatedbrepository.h"
#include "../models/exercisemodel.h"

class ExerciseDbRepository : public TemplateDbRepository<ExerciseModel>
{
public:
    DEFINE_TABLE_NAME(exercises);
    explicit ExerciseDbRepository(QSqlDatabase &database, QObject *parent = nullptr);
};
