#pragma once

#include <dbtoolkit/query/where.h>

inline Where grouped(const Where& condition) { return Where().and_(condition); }

inline void addClause(Where& where, const Where& clause)
{
    where = where.isEmpty() ? grouped(clause) : where.and_(clause);
}

inline Where isNullClause(const QString& column, bool shouldBeNull)
{
    return shouldBeNull ? Where(column).isNull() : Where(column).isNotNull();
}
