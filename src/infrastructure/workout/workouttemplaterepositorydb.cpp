#include "workouttemplaterepositorydb.h"
#include "domain/workout/setprescription.h"
#include "domain/workout/templateexercise.h"
#include "domain/workout/workouttemplatequery.h"
#include "infrastructure/workout/workouttemplateserializer.h"

#include "infrastructure/whereclause.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/createindex.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/select.h>

#include <QHash>

WorkoutTemplateRepositoryDb::WorkoutTemplateRepositoryDb(DbStorage& storage)
    : m_templateRepo(std::make_unique<DbRepository>(
          WorkoutTemplateSerializer::table, WorkoutTemplateSerializer::id_key,
          QStringList { WorkoutTemplateSerializer::id_key, WorkoutTemplateSerializer::name_key,
                        WorkoutTemplateSerializer::notes_key },
          storage, nullptr))
    , m_exerciseRepo(std::make_unique<DbRepository>(
          TemplateExerciseSerializer::table, TemplateExerciseSerializer::id_key,
          QStringList { TemplateExerciseSerializer::id_key,
                        TemplateExerciseSerializer::template_id_key,
                        TemplateExerciseSerializer::definition_id_key,
                        TemplateExerciseSerializer::position_key,
                        TemplateExerciseSerializer::rest_seconds_override_key,
                        TemplateExerciseSerializer::notes_key },
          storage, nullptr))
    , m_prescriptionRepo(std::make_unique<DbRepository>(
          SetPrescriptionSerializer::table, SetPrescriptionSerializer::id_key,
          QStringList {
              SetPrescriptionSerializer::id_key,
              SetPrescriptionSerializer::template_exercise_id_key,
              SetPrescriptionSerializer::metric_key, SetPrescriptionSerializer::load_type_key,
              SetPrescriptionSerializer::repetitions_key, SetPrescriptionSerializer::weight_key,
              SetPrescriptionSerializer::duration_seconds_key,
              SetPrescriptionSerializer::distance_meters_key,
              SetPrescriptionSerializer::rest_seconds_override_key,
              SetPrescriptionSerializer::position_key },
          storage, nullptr))
{
}

WorkoutTemplateRepositoryDb::~WorkoutTemplateRepositoryDb() = default;

bool WorkoutTemplateRepositoryDb::createTables()
{
    CreateTable templates(WorkoutTemplateSerializer::table);
    templates.ifNotExists()
        .column(Column(WorkoutTemplateSerializer::id_key)
                    .integer()
                    .primaryKey()
                    .autoIncrement()
                    .notNull())
        .column(Column(WorkoutTemplateSerializer::name_key).text().notNull())
        .column(Column(WorkoutTemplateSerializer::notes_key).text());

    CreateTable exercises(TemplateExerciseSerializer::table);
    exercises.ifNotExists()
        .column(Column(TemplateExerciseSerializer::id_key)
                    .integer()
                    .primaryKey()
                    .autoIncrement()
                    .notNull())
        .column(Column(TemplateExerciseSerializer::template_id_key).integer().notNull())
        .column(Column(TemplateExerciseSerializer::definition_id_key).integer().notNull())
        .column(Column(TemplateExerciseSerializer::position_key).integer().defaultValue(0))
        .column(Column(TemplateExerciseSerializer::rest_seconds_override_key)
                    .integer()
                    .defaultValue(-1))
        .column(Column(TemplateExerciseSerializer::notes_key).text())
        .foreignKey(TemplateExerciseSerializer::template_id_key, WorkoutTemplateSerializer::table,
                    WorkoutTemplateSerializer::id_key, OnDeleteAction::Cascade);

    CreateTable prescriptions(SetPrescriptionSerializer::table);
    prescriptions.ifNotExists()
        .column(Column(SetPrescriptionSerializer::id_key)
                    .integer()
                    .primaryKey()
                    .autoIncrement()
                    .notNull())
        .column(Column(SetPrescriptionSerializer::template_exercise_id_key).integer().notNull())
        .column(Column(SetPrescriptionSerializer::metric_key)
                    .text()
                    .defaultValue(QStringLiteral("reps")))
        .column(Column(SetPrescriptionSerializer::load_type_key)
                    .text()
                    .defaultValue(QStringLiteral("external")))
        .column(Column(SetPrescriptionSerializer::repetitions_key).integer().defaultValue(0))
        .column(Column(SetPrescriptionSerializer::weight_key).real().defaultValue(0.0))
        .column(Column(SetPrescriptionSerializer::duration_seconds_key).integer().defaultValue(0))
        .column(Column(SetPrescriptionSerializer::distance_meters_key).real().defaultValue(0.0))
        .column(
            Column(SetPrescriptionSerializer::rest_seconds_override_key).integer().defaultValue(-1))
        .column(Column(SetPrescriptionSerializer::position_key).integer().defaultValue(0))
        .foreignKey(SetPrescriptionSerializer::template_exercise_id_key,
                    TemplateExerciseSerializer::table, TemplateExerciseSerializer::id_key,
                    OnDeleteAction::Cascade);

    if (!m_templateRepo->createTable(templates) || !m_exerciseRepo->createTable(exercises)
        || !m_prescriptionRepo->createTable(prescriptions))
        return false;

    QSqlDatabase& database = m_templateRepo->storage().database();

    CreateIndex exercisesByTemplate(QStringLiteral("idx_template_exercises_template"),
                                    TemplateExerciseSerializer::table);
    exercisesByTemplate.ifNotExists().columns(
        QStringList { TemplateExerciseSerializer::template_id_key });

    CreateIndex exercisesByDefinition(QStringLiteral("idx_template_exercises_definition"),
                                      TemplateExerciseSerializer::table);
    exercisesByDefinition.ifNotExists().columns(
        QStringList { TemplateExerciseSerializer::definition_id_key });

    CreateIndex prescriptionsByExercise(QStringLiteral("idx_set_prescriptions_exercise"),
                                        SetPrescriptionSerializer::table);
    prescriptionsByExercise.ifNotExists().columns(
        QStringList { SetPrescriptionSerializer::template_exercise_id_key });

    return exercisesByTemplate.execute(database).toInt() != 0
        && exercisesByDefinition.execute(database).toInt() != 0
        && prescriptionsByExercise.execute(database).toInt() != 0;
}

void WorkoutTemplateRepositoryDb::registerMigrations(MigrationRunner&) { }

std::vector<WorkoutTemplate>
WorkoutTemplateRepositoryDb::findAll(const WorkoutTemplateQuery& query) const
{
    const auto rows
        = m_templateRepo->select(buildWhereClause(query), buildOrderClause(query),
                                 query.limit().value_or(-1), query.offset().value_or(-1));

    std::vector<WorkoutTemplate> results;
    results.reserve(rows.size());
    for (const auto& row : rows)
        results.push_back(WorkoutTemplateSerializer::fromVariant(row));

    loadChildren(results);
    return results;
}

std::optional<WorkoutTemplate>
WorkoutTemplateRepositoryDb::findOne(const WorkoutTemplateQuery& query) const
{
    const auto rows = m_templateRepo->select(buildWhereClause(query), buildOrderClause(query), 1);
    if (rows.isEmpty())
        return std::nullopt;

    std::vector<WorkoutTemplate> results { WorkoutTemplateSerializer::fromVariant(rows.first()) };
    loadChildren(results);
    return results.front();
}

int WorkoutTemplateRepositoryDb::save(const WorkoutTemplate& workoutTemplate)
{
    DbStorage& storage = m_templateRepo->storage();
    storage.beginTransaction();

    const QVariantMap data = WorkoutTemplateSerializer::toVariant(workoutTemplate);

    int templateId = workoutTemplate.id();
    if (templateId != -1
        && m_templateRepo->exists(Where(WorkoutTemplateSerializer::id_key).equals(templateId)))
    {
        m_templateRepo->update(data, Where(WorkoutTemplateSerializer::id_key).equals(templateId));
    }
    else
    {
        templateId = m_templateRepo->insert(data).toInt();
    }

    saveChildren(templateId, workoutTemplate);

    storage.commit();
    return templateId;
}

bool WorkoutTemplateRepositoryDb::remove(const WorkoutTemplateQuery& query)
{
    return m_templateRepo->remove(buildWhereClause(query)) > 0;
}

int WorkoutTemplateRepositoryDb::count(const WorkoutTemplateQuery& query) const
{
    return m_templateRepo->count(buildWhereClause(query));
}

bool WorkoutTemplateRepositoryDb::exists(const WorkoutTemplateQuery& query) const
{
    return m_templateRepo->exists(buildWhereClause(query));
}

void WorkoutTemplateRepositoryDb::loadChildren(std::vector<WorkoutTemplate>& templates) const
{
    if (templates.empty())
        return;

    QList<int> templateIds;
    templateIds.reserve(static_cast<int>(templates.size()));
    for (const auto& workoutTemplate : templates)
        templateIds.append(workoutTemplate.id());

    const auto exerciseRows
        = m_exerciseRepo->select(Where(TemplateExerciseSerializer::template_id_key).in(templateIds),
                                 Order(TemplateExerciseSerializer::template_id_key)
                                     .asc()
                                     .then(TemplateExerciseSerializer::position_key)
                                     .asc());

    QList<int> exerciseIds;
    exerciseIds.reserve(exerciseRows.size());
    for (const auto& row : exerciseRows)
        exerciseIds.append(row.value(TemplateExerciseSerializer::id_key).toInt());

    QHash<int, std::vector<SetPrescription>> prescriptionsByExercise;
    if (!exerciseIds.isEmpty())
    {
        const auto prescriptionRows = m_prescriptionRepo->select(
            Where(SetPrescriptionSerializer::template_exercise_id_key).in(exerciseIds),
            Order(SetPrescriptionSerializer::template_exercise_id_key)
                .asc()
                .then(SetPrescriptionSerializer::position_key)
                .asc());

        for (const auto& row : prescriptionRows)
        {
            const int exerciseId
                = row.value(SetPrescriptionSerializer::template_exercise_id_key).toInt();
            prescriptionsByExercise[exerciseId].push_back(
                SetPrescriptionSerializer::fromVariant(row));
        }
    }

    QHash<int, std::vector<TemplateExercise>> exercisesByTemplate;
    for (const auto& row : exerciseRows)
    {
        TemplateExercise exercise = TemplateExerciseSerializer::fromVariant(row);

        const auto it
            = prescriptionsByExercise.find(row.value(TemplateExerciseSerializer::id_key).toInt());
        if (it != prescriptionsByExercise.end())
        {
            for (const auto& prescription : it.value())
                exercise.addSet(prescription);
        }

        exercisesByTemplate[row.value(TemplateExerciseSerializer::template_id_key).toInt()]
            .push_back(std::move(exercise));
    }

    for (auto& workoutTemplate : templates)
    {
        const auto it = exercisesByTemplate.find(workoutTemplate.id());
        if (it == exercisesByTemplate.end())
            continue;
        for (const auto& exercise : it.value())
            workoutTemplate.addExercise(exercise);
    }
}

void WorkoutTemplateRepositoryDb::saveChildren(int templateId,
                                               const WorkoutTemplate& workoutTemplate)
{
    m_exerciseRepo->remove(Where(TemplateExerciseSerializer::template_id_key).equals(templateId));

    for (const auto& exercise : workoutTemplate.exercises())
    {
        const int exerciseId
            = m_exerciseRepo->insert(TemplateExerciseSerializer::toVariant(exercise, templateId))
                  .toInt();

        for (const auto& prescription : exercise.sets())
        {
            m_prescriptionRepo->insert(
                SetPrescriptionSerializer::toVariant(prescription, exerciseId));
        }
    }
}

Where WorkoutTemplateRepositoryDb::buildWhereClause(const WorkoutTemplateQuery& query) const
{
    Where where;

    if (query.id().has_value())
        addClause(where, Where(WorkoutTemplateSerializer::id_key).equals(query.id().value()));

    if (query.name().has_value())
        addClause(where, Where(WorkoutTemplateSerializer::name_key).equals(query.name().value()));

    if (query.nameContains().has_value())
    {
        addClause(where,
                  Where(WorkoutTemplateSerializer::name_key)
                      .like(QStringLiteral("%%1%").arg(query.nameContains().value())));
    }

    if (query.referencesDefinition().has_value())
    {
        const Select subquery = Select(QStringList { TemplateExerciseSerializer::template_id_key })
                                    .from(TemplateExerciseSerializer::table)
                                    .where(Where(TemplateExerciseSerializer::definition_id_key)
                                               .equals(query.referencesDefinition().value()));

        addClause(where, Where(WorkoutTemplateSerializer::id_key).in(subquery));
    }

    return where;
}

Order WorkoutTemplateRepositoryDb::buildOrderClause(const WorkoutTemplateQuery& query) const
{
    if (!query.orderByNameDirection().has_value())
        return Order();

    Order order(WorkoutTemplateSerializer::name_key);
    return query.orderByNameDirection().value() == SortDirection::Ascending ? order.asc()
                                                                            : order.desc();
}
