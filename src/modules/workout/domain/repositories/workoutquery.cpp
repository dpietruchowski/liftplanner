#include "workoutquery.h"

WorkoutQuery &WorkoutQuery::whereId(int id)
{
    m_id = id;
    return *this;
}
WorkoutQuery &WorkoutQuery::whereName(const QString &name)
{
    m_name = name;
    return *this;
}
WorkoutQuery &WorkoutQuery::whereCreatedAfter(const QDateTime &dateTime)
{
    m_createdAfter = dateTime;
    return *this;
}
WorkoutQuery &WorkoutQuery::whereCreatedBefore(const QDateTime &dateTime)
{
    m_createdBefore = dateTime;
    return *this;
}
WorkoutQuery &WorkoutQuery::whereStartedTimeIsNull()
{
    m_startedTimeIsNull = true;
    return *this;
}
WorkoutQuery &WorkoutQuery::whereStartedTimeIsNotNull()
{
    m_startedTimeIsNull = false;
    return *this;
}
WorkoutQuery &WorkoutQuery::wherePlannedTimeIsNull()
{
    m_plannedTimeIsNull = true;
    return *this;
}
WorkoutQuery &WorkoutQuery::wherePlannedTimeIsNotNull()
{
    m_plannedTimeIsNull = false;
    return *this;
}
WorkoutQuery &WorkoutQuery::orderByCreatedTime(SortDirection direction)
{
    m_orderByCreatedTimeDirection = direction;
    return *this;
}
WorkoutQuery &WorkoutQuery::orderByStartedTime(SortDirection direction)
{
    m_orderByStartedTimeDirection = direction;
    return *this;
}
WorkoutQuery &WorkoutQuery::orderByPlannedTime(SortDirection direction)
{
    m_orderByPlannedTimeDirection = direction;
    return *this;
}
WorkoutQuery &WorkoutQuery::withLimit(int limit)
{
    m_limit = limit;
    return *this;
}
WorkoutQuery &WorkoutQuery::withOffset(int offset)
{
    m_offset = offset;
    return *this;
}

const std::optional<int> &WorkoutQuery::id() const { return m_id; }
const std::optional<QString> &WorkoutQuery::name() const { return m_name; }
const std::optional<QDateTime> &WorkoutQuery::createdAfter() const { return m_createdAfter; }
const std::optional<QDateTime> &WorkoutQuery::createdBefore() const { return m_createdBefore; }
const std::optional<bool> &WorkoutQuery::startedTimeIsNull() const { return m_startedTimeIsNull; }
const std::optional<bool> &WorkoutQuery::plannedTimeIsNull() const { return m_plannedTimeIsNull; }
const std::optional<SortDirection> &WorkoutQuery::orderByCreatedTimeDirection() const { return m_orderByCreatedTimeDirection; }
const std::optional<SortDirection> &WorkoutQuery::orderByStartedTimeDirection() const { return m_orderByStartedTimeDirection; }
const std::optional<SortDirection> &WorkoutQuery::orderByPlannedTimeDirection() const { return m_orderByPlannedTimeDirection; }
const std::optional<int> &WorkoutQuery::limit() const { return m_limit; }
const std::optional<int> &WorkoutQuery::offset() const { return m_offset; }
