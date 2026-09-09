#pragma once

#include "domain/workout/workouttemplate.h"
#include "domain/workout/workouttemplaterepository.h"

#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/where.h>

#include <QList>
#include <QVariantMap>
#include <QVector>
#include <memory>
#include <vector>

class DbStorage;
class DbRepository;
class MigrationRunner;

class WorkoutTemplateRepositoryDb : public WorkoutTemplateRepository
{
public:
    explicit WorkoutTemplateRepositoryDb(DbStorage& storage);
    ~WorkoutTemplateRepositoryDb() override;

    bool createTables();
    void registerMigrations(MigrationRunner& runner);

    std::optional<WorkoutTemplate> findOne(const WorkoutTemplateQuery& query) const override;
    int save(const WorkoutTemplate& workoutTemplate) override;
    bool remove(const WorkoutTemplateQuery& query) override;
    int count(const WorkoutTemplateQuery& query) const override;
    bool exists(const WorkoutTemplateQuery& query) const override;

private:
    void loadChildren(std::vector<WorkoutTemplate>& templates) const;
    void saveChildren(int templateId, const WorkoutTemplate& workoutTemplate);

    Where buildWhereClause(const WorkoutTemplateQuery& query) const;
    Order buildOrderClause(const WorkoutTemplateQuery& query) const;

    std::unique_ptr<DbRepository> m_templateRepo;
    std::unique_ptr<DbRepository> m_exerciseRepo;
    std::unique_ptr<DbRepository> m_prescriptionRepo;
};
