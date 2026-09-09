#include "workouttemplatequery.h"

WorkoutTemplateQuery& WorkoutTemplateQuery::whereId(int id)
{
    m_id = id;
    return *this;
}

WorkoutTemplateQuery& WorkoutTemplateQuery::whereName(const QString& name)
{
    m_name = name;
    return *this;
}

WorkoutTemplateQuery& WorkoutTemplateQuery::whereNameContains(const QString& text)
{
    m_nameContains = text;
    return *this;
}

WorkoutTemplateQuery& WorkoutTemplateQuery::whereReferencesDefinition(int definitionId)
{
    m_referencesDefinition = definitionId;
    return *this;
}

WorkoutTemplateQuery& WorkoutTemplateQuery::orderByName(SortDirection direction)
{
    m_orderByNameDirection = direction;
    return *this;
}

WorkoutTemplateQuery& WorkoutTemplateQuery::withLimit(int limit)
{
    m_limit = limit;
    return *this;
}

WorkoutTemplateQuery& WorkoutTemplateQuery::withOffset(int offset)
{
    m_offset = offset;
    return *this;
}

const std::optional<int>& WorkoutTemplateQuery::id() const { return m_id; }
const std::optional<QString>& WorkoutTemplateQuery::name() const { return m_name; }
const std::optional<QString>& WorkoutTemplateQuery::nameContains() const { return m_nameContains; }
const std::optional<int>& WorkoutTemplateQuery::referencesDefinition() const
{
    return m_referencesDefinition;
}
const std::optional<SortDirection>& WorkoutTemplateQuery::orderByNameDirection() const
{
    return m_orderByNameDirection;
}
const std::optional<int>& WorkoutTemplateQuery::limit() const { return m_limit; }
const std::optional<int>& WorkoutTemplateQuery::offset() const { return m_offset; }
