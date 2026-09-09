#pragma once

#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/where.h>

class WorkoutTemplateQuery;

namespace WorkoutTemplateConditions
{

Where where(const WorkoutTemplateQuery& query);
Order order(const WorkoutTemplateQuery& query, const QString& table = QString());

}
