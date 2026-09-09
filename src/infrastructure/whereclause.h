#pragma once

#include <dbtoolkit/query/where.h>

inline Where grouped(const Where& condition) { return Where().and_(condition); }

inline void addClause(Where& where, const Where& clause)
{
    where = where.isEmpty() ? grouped(clause) : where.and_(clause);
}
