#pragma once

#include "domain/sortdirection.h"
#include <QString>
#include <optional>

class WorkoutTemplateQuery final
{
public:
    WorkoutTemplateQuery& whereId(int id);
    WorkoutTemplateQuery& whereName(const QString& name);
    WorkoutTemplateQuery& whereNameContains(const QString& text);
    WorkoutTemplateQuery& whereReferencesDefinition(int definitionId);
    WorkoutTemplateQuery& orderByName(SortDirection direction);
    WorkoutTemplateQuery& withLimit(int limit);
    WorkoutTemplateQuery& withOffset(int offset);

    const std::optional<int>& id() const;
    const std::optional<QString>& name() const;
    const std::optional<QString>& nameContains() const;
    const std::optional<int>& referencesDefinition() const;
    const std::optional<SortDirection>& orderByNameDirection() const;
    const std::optional<int>& limit() const;
    const std::optional<int>& offset() const;

private:
    std::optional<int> m_id;
    std::optional<QString> m_name;
    std::optional<QString> m_nameContains;
    std::optional<int> m_referencesDefinition;
    std::optional<SortDirection> m_orderByNameDirection;
    std::optional<int> m_limit;
    std::optional<int> m_offset;
};
