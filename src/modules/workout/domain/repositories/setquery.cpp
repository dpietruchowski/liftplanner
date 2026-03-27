#include "setquery.h"

SetQuery &SetQuery::whereId(int id)
{
    m_id = id;
    return *this;
}
SetQuery &SetQuery::whereExerciseId(int exerciseId)
{
    m_exerciseId = exerciseId;
    return *this;
}
SetQuery &SetQuery::withLimit(int limit)
{
    m_limit = limit;
    return *this;
}

const std::optional<int> &SetQuery::id() const { return m_id; }
const std::optional<int> &SetQuery::exerciseId() const { return m_exerciseId; }
const std::optional<int> &SetQuery::limit() const { return m_limit; }
