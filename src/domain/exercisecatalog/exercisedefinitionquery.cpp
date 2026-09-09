#include "exercisedefinitionquery.h"

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereId(int id)
{
    m_id = id;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereSlug(const QString& slug)
{
    m_slug = slug;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereNameContains(const QString& text)
{
    m_nameContains = text;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereKind(ExerciseKind kind)
{
    m_kind = kind;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereEquipment(Equipment equipment)
{
    m_equipment = equipment;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereMuscle(Muscle muscle)
{
    m_muscle = muscle;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereRegion(BodyRegion region)
{
    m_region = region;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereOrigin(CatalogOrigin origin)
{
    m_origin = origin;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::whereArchived(bool archived)
{
    m_archived = archived;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::orderByName(SortDirection direction)
{
    m_orderByNameDirection = direction;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::withLimit(int limit)
{
    m_limit = limit;
    return *this;
}

ExerciseDefinitionQuery& ExerciseDefinitionQuery::withOffset(int offset)
{
    m_offset = offset;
    return *this;
}

const std::optional<int>& ExerciseDefinitionQuery::id() const { return m_id; }
const std::optional<QString>& ExerciseDefinitionQuery::slug() const { return m_slug; }
const std::optional<QString>& ExerciseDefinitionQuery::nameContains() const
{
    return m_nameContains;
}
const std::optional<ExerciseKind>& ExerciseDefinitionQuery::kind() const { return m_kind; }
const std::optional<Equipment>& ExerciseDefinitionQuery::equipment() const { return m_equipment; }
const std::optional<Muscle>& ExerciseDefinitionQuery::muscle() const { return m_muscle; }
const std::optional<BodyRegion>& ExerciseDefinitionQuery::region() const { return m_region; }
const std::optional<CatalogOrigin>& ExerciseDefinitionQuery::origin() const { return m_origin; }
const std::optional<bool>& ExerciseDefinitionQuery::archived() const { return m_archived; }
const std::optional<SortDirection>& ExerciseDefinitionQuery::orderByNameDirection() const
{
    return m_orderByNameDirection;
}
const std::optional<int>& ExerciseDefinitionQuery::limit() const { return m_limit; }
const std::optional<int>& ExerciseDefinitionQuery::offset() const { return m_offset; }
