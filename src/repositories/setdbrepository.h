#pragma once
#include "templatedbrepository.h"
#include "../models/setmodel.h"

class SetDbRepository : public TemplateDbRepository<SetModel>
{
public:
    DEFINE_TABLE_NAME(sets);
    explicit SetDbRepository(QSqlDatabase &database, QObject *parent = nullptr);
};
