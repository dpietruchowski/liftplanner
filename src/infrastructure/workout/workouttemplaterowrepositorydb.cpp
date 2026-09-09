#include "workouttemplaterowrepositorydb.h"

#include "domain/workout/workouttemplatequery.h"
#include "infrastructure/exercisecatalog/exercisedefinitionserializer.h"
#include "infrastructure/workout/workouttemplateconditions.h"
#include "infrastructure/workout/workouttemplateserializer.h"

#include <dbtoolkit/dbprojection.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/query/alias.h>
#include <dbtoolkit/query/expr.h>
#include <dbtoolkit/query/join.h>
#include <dbtoolkit/query/projection.h>
#include <dbtoolkit/query/select.h>

namespace
{

const TableAlias templates("t");
const TableAlias exercises("te");
const TableAlias definitions("ed");

constexpr const char* set_count_key = "set_count";

struct TemplateExerciseRow
{
    int templateId { -1 };
    QString templateName;
    QString templateNotes;
    bool hasExercise { false };
    bool hasDefinition { false };
    QString definitionName;
    int setCount { 0 };
};

QString prescriptionCountSql()
{
    return QStringLiteral("(SELECT COUNT(*) FROM %1 WHERE %1.%2 = %3.%4)")
        .arg(SetPrescriptionSerializer::table, SetPrescriptionSerializer::template_exercise_id_key,
             exercises.prefix(), TemplateExerciseSerializer::id_key);
}

QList<Projection> projections()
{
    return { Projection(templates,
                        { WorkoutTemplateSerializer::id_key, WorkoutTemplateSerializer::name_key,
                          WorkoutTemplateSerializer::notes_key }),
             Projection(exercises, { TemplateExerciseSerializer::id_key }),
             Projection(definitions, { ExerciseDefinitionSerializer::name_key }),
             Projection(QStringLiteral("calc"), { Expr(prescriptionCountSql(), set_count_key) }) };
}

TemplateExerciseRow toRow(const ProjectedRow& row)
{
    const QVariantMap templateGroup = row.of(templates.prefix());
    const QVariantMap exerciseGroup = row.of(exercises.prefix());
    const QVariantMap definitionGroup = row.of(definitions.prefix());
    const QVariantMap calculated = row.of(QStringLiteral("calc"));

    TemplateExerciseRow result;
    result.templateId = templateGroup.value(WorkoutTemplateSerializer::id_key).toInt();
    result.templateName = templateGroup.value(WorkoutTemplateSerializer::name_key).toString();
    result.templateNotes = templateGroup.value(WorkoutTemplateSerializer::notes_key).toString();
    result.hasExercise = !exerciseGroup.value(TemplateExerciseSerializer::id_key).isNull();
    result.hasDefinition = !definitionGroup.value(ExerciseDefinitionSerializer::name_key).isNull();
    result.definitionName
        = definitionGroup.value(ExerciseDefinitionSerializer::name_key).toString();
    result.setCount = calculated.value(set_count_key).toInt();
    return result;
}

Select selectedTemplates(const WorkoutTemplateQuery& query)
{
    Select selected(QStringList { WorkoutTemplateSerializer::id_key });
    selected.from(WorkoutTemplateSerializer::table)
        .where(WorkoutTemplateConditions::where(query))
        .orderBy(WorkoutTemplateConditions::order(query));

    if (query.limit().has_value())
        selected.limit(query.limit().value());
    if (query.offset().has_value())
        selected.offset(query.offset().value());

    return selected;
}

Select shape(const WorkoutTemplateQuery& query)
{
    Select select;
    select.from(WorkoutTemplateSerializer::table)
        .as(templates)
        .leftJoin(Join(TemplateExerciseSerializer::table)
                      .as(exercises)
                      .on(templates, WorkoutTemplateSerializer::id_key)
                      .equals(TemplateExerciseSerializer::template_id_key))
        .leftJoin(Join(ExerciseDefinitionSerializer::table)
                      .as(definitions)
                      .on(exercises, TemplateExerciseSerializer::definition_id_key)
                      .equals(ExerciseDefinitionSerializer::id_key))
        .where(Where(templates.createColumn(WorkoutTemplateSerializer::id_key))
                   .in(selectedTemplates(query)));

    Order order = WorkoutTemplateConditions::order(query, templates.prefix());
    if (order.isEmpty())
        order = Order(templates.createColumn(WorkoutTemplateSerializer::id_key)).asc();
    order.then(exercises.createColumn(TemplateExerciseSerializer::position_key)).asc();

    return select.orderBy(order);
}

}

WorkoutTemplateRowRepositoryDb::WorkoutTemplateRowRepositoryDb(DbStorage& storage)
    : m_storage(storage)
{
}

std::vector<WorkoutTemplateRow>
WorkoutTemplateRowRepositoryDb::findAll(const WorkoutTemplateQuery& query) const
{
    const DbProjection<TemplateExerciseRow> projection(m_storage, projections(), &toRow);

    std::vector<WorkoutTemplateRow> results;
    for (const TemplateExerciseRow& row : projection.findAll(shape(query)))
    {
        if (results.empty() || results.back().id != row.templateId)
        {
            WorkoutTemplateRow entry;
            entry.id = row.templateId;
            entry.name = row.templateName;
            entry.notes = row.templateNotes;
            results.push_back(entry);
        }

        if (!row.hasExercise)
            continue;

        WorkoutTemplateRow& entry = results.back();
        entry.exerciseCount += 1;
        entry.setCount += row.setCount;

        if (row.hasDefinition)
            entry.exerciseNames.append(row.definitionName);
        else
            entry.complete = false;
    }

    return results;
}
