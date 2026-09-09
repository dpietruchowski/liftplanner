#include "workouttemplateconditions.h"

#include "domain/workout/workouttemplatequery.h"
#include "infrastructure/whereclause.h"
#include "infrastructure/workout/workouttemplateserializer.h"

#include <dbtoolkit/query/select.h>

namespace WorkoutTemplateConditions
{

Where where(const WorkoutTemplateQuery& query)
{
    Where condition;

    if (query.id().has_value())
        addClause(condition, Where(WorkoutTemplateSerializer::id_key).equals(query.id().value()));

    if (query.name().has_value())
    {
        addClause(condition,
                  Where(WorkoutTemplateSerializer::name_key).equals(query.name().value()));
    }

    if (query.nameContains().has_value())
    {
        addClause(condition,
                  Where(WorkoutTemplateSerializer::name_key)
                      .like(QStringLiteral("%%1%").arg(query.nameContains().value())));
    }

    if (query.referencesDefinition().has_value())
    {
        const Select subquery = Select(QStringList { TemplateExerciseSerializer::template_id_key })
                                    .from(TemplateExerciseSerializer::table)
                                    .where(Where(TemplateExerciseSerializer::definition_id_key)
                                               .equals(query.referencesDefinition().value()));

        addClause(condition, Where(WorkoutTemplateSerializer::id_key).in(subquery));
    }

    return condition;
}

Order order(const WorkoutTemplateQuery& query, const QString& table)
{
    if (!query.orderByNameDirection().has_value())
        return Order();

    const QString column = table.isEmpty()
        ? QString::fromLatin1(WorkoutTemplateSerializer::name_key)
        : QStringLiteral("%1.%2").arg(table, WorkoutTemplateSerializer::name_key);

    Order clause(column);
    return query.orderByNameDirection().value() == SortDirection::Ascending ? clause.asc()
                                                                            : clause.desc();
}

}
