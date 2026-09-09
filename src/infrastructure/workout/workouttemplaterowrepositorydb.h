#pragma once

#include "domain/workout/workouttemplaterow.h"
#include "domain/workout/workouttemplaterowrepository.h"

#include <vector>

class DbStorage;

class WorkoutTemplateRowRepositoryDb : public WorkoutTemplateRowRepository
{
public:
    explicit WorkoutTemplateRowRepositoryDb(DbStorage& storage);

    std::vector<WorkoutTemplateRow> findAll(const WorkoutTemplateQuery& query) const override;

private:
    DbStorage& m_storage;
};
