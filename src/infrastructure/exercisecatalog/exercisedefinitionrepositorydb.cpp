#include "exercisedefinitionrepositorydb.h"
#include "domain/exercisecatalog/exercisedefinitionquery.h"
#include "infrastructure/exercisecatalog/exercisedefinitionserializer.h"
#include "infrastructure/exercisecatalog/muscleinvolvementserializer.h"

#include "infrastructure/whereclause.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/migrationrunner.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/createindex.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/select.h>
#include <dbtoolkit/query/where.h>

#include <QHash>

namespace
{

QStringList muscleKeysOfRegion(BodyRegion region)
{
    QStringList keys;
    for (const MuscleEntry& entry : muscleEntries)
    {
        if (entry.region == region)
            keys.append(QString::fromLatin1(entry.key));
    }
    return keys;
}

Where definitionIdsTargeting(const Where& muscleCondition)
{
    const Select subquery = Select(QStringList { MuscleInvolvementSerializer::definition_id_key })
                                .from(MuscleInvolvementSerializer::table)
                                .where(muscleCondition);

    return Where(ExerciseDefinitionSerializer::id_key).in(subquery);
}

}

ExerciseDefinitionRepositoryDb::ExerciseDefinitionRepositoryDb(DbStorage& storage)
    : m_definitionRepo(std::make_unique<DbRepository>(
          ExerciseDefinitionSerializer::table, ExerciseDefinitionSerializer::id_key,
          QStringList {
              ExerciseDefinitionSerializer::id_key, ExerciseDefinitionSerializer::slug_key,
              ExerciseDefinitionSerializer::name_key, ExerciseDefinitionSerializer::aliases_key,
              ExerciseDefinitionSerializer::kind_key,
              ExerciseDefinitionSerializer::default_load_type_key,
              ExerciseDefinitionSerializer::default_metric_key,
              ExerciseDefinitionSerializer::equipment_key,
              ExerciseDefinitionSerializer::mechanics_key,
              ExerciseDefinitionSerializer::laterality_key,
              ExerciseDefinitionSerializer::default_rest_seconds_key,
              ExerciseDefinitionSerializer::video_url_key,
              ExerciseDefinitionSerializer::instructions_key,
              ExerciseDefinitionSerializer::origin_key,
              ExerciseDefinitionSerializer::archived_key },
          storage, nullptr))
    , m_muscleRepo(std::make_unique<DbRepository>(
          MuscleInvolvementSerializer::table, MuscleInvolvementSerializer::id_key,
          QStringList {
              MuscleInvolvementSerializer::id_key, MuscleInvolvementSerializer::definition_id_key,
              MuscleInvolvementSerializer::muscle_key, MuscleInvolvementSerializer::role_key,
              MuscleInvolvementSerializer::position_key },
          storage, nullptr))
{
}

ExerciseDefinitionRepositoryDb::~ExerciseDefinitionRepositoryDb() = default;

bool ExerciseDefinitionRepositoryDb::createTables()
{
    CreateTable definitions(ExerciseDefinitionSerializer::table);
    definitions.ifNotExists()
        .column(Column(ExerciseDefinitionSerializer::id_key)
                    .integer()
                    .primaryKey()
                    .autoIncrement()
                    .notNull())
        .column(Column(ExerciseDefinitionSerializer::slug_key).text().notNull().unique())
        .column(Column(ExerciseDefinitionSerializer::name_key).text().notNull())
        .column(Column(ExerciseDefinitionSerializer::aliases_key).text())
        .column(Column(ExerciseDefinitionSerializer::kind_key)
                    .text()
                    .defaultValue(QStringLiteral("strength")))
        .column(Column(ExerciseDefinitionSerializer::default_load_type_key)
                    .text()
                    .defaultValue(QStringLiteral("external")))
        .column(Column(ExerciseDefinitionSerializer::default_metric_key)
                    .text()
                    .defaultValue(QStringLiteral("reps")))
        .column(Column(ExerciseDefinitionSerializer::equipment_key)
                    .text()
                    .defaultValue(QStringLiteral("other")))
        .column(Column(ExerciseDefinitionSerializer::mechanics_key)
                    .text()
                    .defaultValue(QStringLiteral("compound")))
        .column(Column(ExerciseDefinitionSerializer::laterality_key)
                    .text()
                    .defaultValue(QStringLiteral("bilateral")))
        .column(Column(ExerciseDefinitionSerializer::default_rest_seconds_key)
                    .integer()
                    .defaultValue(120))
        .column(Column(ExerciseDefinitionSerializer::video_url_key).text())
        .column(Column(ExerciseDefinitionSerializer::instructions_key).text())
        .column(Column(ExerciseDefinitionSerializer::origin_key)
                    .text()
                    .defaultValue(QStringLiteral("built_in")))
        .column(Column(ExerciseDefinitionSerializer::archived_key).integer().defaultValue(0));

    CreateTable muscles(MuscleInvolvementSerializer::table);
    muscles.ifNotExists()
        .column(Column(MuscleInvolvementSerializer::id_key)
                    .integer()
                    .primaryKey()
                    .autoIncrement()
                    .notNull())
        .column(Column(MuscleInvolvementSerializer::definition_id_key).integer().notNull())
        .column(Column(MuscleInvolvementSerializer::muscle_key).text().notNull())
        .column(Column(MuscleInvolvementSerializer::role_key).text().notNull())
        .column(Column(MuscleInvolvementSerializer::position_key).integer().defaultValue(0))
        .foreignKey(MuscleInvolvementSerializer::definition_id_key,
                    ExerciseDefinitionSerializer::table, ExerciseDefinitionSerializer::id_key,
                    OnDeleteAction::Cascade);

    if (!m_definitionRepo->createTable(definitions) || !m_muscleRepo->createTable(muscles))
        return false;

    QSqlDatabase& database = m_definitionRepo->storage().database();

    CreateIndex musclesByDefinition(QStringLiteral("idx_definition_muscles_definition"),
                                    MuscleInvolvementSerializer::table);
    musclesByDefinition.ifNotExists().columns(
        QStringList { MuscleInvolvementSerializer::definition_id_key });

    CreateIndex musclesByMuscle(QStringLiteral("idx_definition_muscles_muscle"),
                                MuscleInvolvementSerializer::table);
    musclesByMuscle.ifNotExists().columns(QStringList { MuscleInvolvementSerializer::muscle_key });

    return musclesByDefinition.execute(database).toInt() != 0
        && musclesByMuscle.execute(database).toInt() != 0;
}

void ExerciseDefinitionRepositoryDb::registerMigrations(MigrationRunner&) { }

std::vector<ExerciseDefinition>
ExerciseDefinitionRepositoryDb::findAll(const ExerciseDefinitionQuery& query) const
{
    const auto rows
        = m_definitionRepo->select(buildWhereClause(query), buildOrderClause(query),
                                   query.limit().value_or(-1), query.offset().value_or(-1));

    std::vector<ExerciseDefinition> results;
    results.reserve(rows.size());
    for (const auto& row : rows)
        results.push_back(ExerciseDefinitionSerializer::fromVariant(row));

    loadMuscles(results);
    return results;
}

std::optional<ExerciseDefinition>
ExerciseDefinitionRepositoryDb::findOne(const ExerciseDefinitionQuery& query) const
{
    const auto rows = m_definitionRepo->select(buildWhereClause(query), buildOrderClause(query), 1,
                                               query.offset().value_or(-1));
    if (rows.isEmpty())
        return std::nullopt;

    ExerciseDefinition definition = ExerciseDefinitionSerializer::fromVariant(rows.first());
    loadMuscles(definition);
    return definition;
}

int ExerciseDefinitionRepositoryDb::save(const ExerciseDefinition& definition)
{
    DbStorage& storage = m_definitionRepo->storage();
    storage.beginTransaction();

    QVariantMap data = ExerciseDefinitionSerializer::toVariant(definition);

    int definitionId = definition.id();
    if (definitionId == -1)
        definitionId = findIdBySlug(definition.slug());

    if (definitionId != -1)
    {
        data.insert(ExerciseDefinitionSerializer::id_key, definitionId);
        m_definitionRepo->update(data,
                                 Where(ExerciseDefinitionSerializer::id_key).equals(definitionId));
    }
    else
    {
        definitionId = m_definitionRepo->insert(data).toInt();
    }

    saveMuscles(definitionId, definition);

    storage.commit();
    return definitionId;
}

bool ExerciseDefinitionRepositoryDb::remove(const ExerciseDefinitionQuery& query)
{
    return m_definitionRepo->remove(buildWhereClause(query)) > 0;
}

int ExerciseDefinitionRepositoryDb::count(const ExerciseDefinitionQuery& query) const
{
    return m_definitionRepo->count(buildWhereClause(query));
}

bool ExerciseDefinitionRepositoryDb::exists(const ExerciseDefinitionQuery& query) const
{
    return m_definitionRepo->exists(buildWhereClause(query));
}

int ExerciseDefinitionRepositoryDb::findIdBySlug(const QString& slug) const
{
    if (slug.isEmpty())
        return -1;

    const auto rows = m_definitionRepo->select(
        Where(ExerciseDefinitionSerializer::slug_key).equals(slug), Order(), 1);
    if (rows.isEmpty())
        return -1;

    return rows.first().value(ExerciseDefinitionSerializer::id_key).toInt();
}

void ExerciseDefinitionRepositoryDb::loadMuscles(ExerciseDefinition& definition) const
{
    const auto rows = m_muscleRepo->select(
        Where(MuscleInvolvementSerializer::definition_id_key).equals(definition.id()),
        Order(MuscleInvolvementSerializer::position_key).asc());

    for (const auto& row : rows)
        definition.addMuscle(MuscleInvolvementSerializer::fromVariant(row));
}

void ExerciseDefinitionRepositoryDb::loadMuscles(std::vector<ExerciseDefinition>& definitions) const
{
    if (definitions.empty())
        return;

    QList<int> definitionIds;
    definitionIds.reserve(static_cast<int>(definitions.size()));
    for (const auto& definition : definitions)
        definitionIds.append(definition.id());

    const auto rows = m_muscleRepo->select(
        Where(MuscleInvolvementSerializer::definition_id_key).in(definitionIds),
        Order(MuscleInvolvementSerializer::definition_id_key)
            .asc()
            .then(MuscleInvolvementSerializer::position_key)
            .asc());

    QHash<int, std::vector<MuscleInvolvement>> musclesByDefinition;
    for (const auto& row : rows)
    {
        const int definitionId = row.value(MuscleInvolvementSerializer::definition_id_key).toInt();
        musclesByDefinition[definitionId].push_back(MuscleInvolvementSerializer::fromVariant(row));
    }

    for (auto& definition : definitions)
    {
        const auto it = musclesByDefinition.find(definition.id());
        if (it == musclesByDefinition.end())
            continue;
        for (const auto& involvement : it.value())
            definition.addMuscle(involvement);
    }
}

void ExerciseDefinitionRepositoryDb::saveMuscles(int definitionId,
                                                 const ExerciseDefinition& definition)
{
    m_muscleRepo->remove(
        Where(MuscleInvolvementSerializer::definition_id_key).equals(definitionId));

    const auto& muscles = definition.muscles();
    for (size_t i = 0; i < muscles.size(); ++i)
    {
        m_muscleRepo->insert(
            MuscleInvolvementSerializer::toVariant(muscles[i], definitionId, static_cast<int>(i)));
    }
}

Where ExerciseDefinitionRepositoryDb::buildWhereClause(const ExerciseDefinitionQuery& query) const
{
    Where where;

    if (query.id().has_value())
        addClause(where, Where(ExerciseDefinitionSerializer::id_key).equals(query.id().value()));

    if (query.slug().has_value())
        addClause(where,
                  Where(ExerciseDefinitionSerializer::slug_key).equals(query.slug().value()));

    if (query.nameContains().has_value())
    {
        const QString pattern = QStringLiteral("%%1%").arg(query.nameContains().value());
        addClause(where,
                  Where(ExerciseDefinitionSerializer::name_key)
                      .like(pattern)
                      .or_(Where(ExerciseDefinitionSerializer::aliases_key).like(pattern)));
    }

    if (query.kind().has_value())
        addClause(where,
                  Where(ExerciseDefinitionSerializer::kind_key)
                      .equals(exerciseKindToString(query.kind().value())));

    if (query.equipment().has_value())
        addClause(where,
                  Where(ExerciseDefinitionSerializer::equipment_key)
                      .equals(equipmentToString(query.equipment().value())));

    if (query.muscle().has_value())
    {
        addClause(where,
                  definitionIdsTargeting(Where(MuscleInvolvementSerializer::muscle_key)
                                             .equals(muscleToString(query.muscle().value()))));
    }

    if (query.region().has_value())
    {
        addClause(where,
                  definitionIdsTargeting(Where(MuscleInvolvementSerializer::muscle_key)
                                             .in(muscleKeysOfRegion(query.region().value()))));
    }

    if (query.origin().has_value())
        addClause(where,
                  Where(ExerciseDefinitionSerializer::origin_key)
                      .equals(catalogOriginToString(query.origin().value())));

    if (query.archived().has_value())
        addClause(where,
                  Where(ExerciseDefinitionSerializer::archived_key)
                      .equals(query.archived().value() ? 1 : 0));

    return where;
}

Order ExerciseDefinitionRepositoryDb::buildOrderClause(const ExerciseDefinitionQuery& query) const
{
    if (!query.orderByNameDirection().has_value())
        return Order();

    Order order(ExerciseDefinitionSerializer::name_key);
    return query.orderByNameDirection().value() == SortDirection::Ascending ? order.asc()
                                                                            : order.desc();
}
