#pragma once

#include "core/types.h"
#include <optional>
#include <QDateTime>

class WorkoutQuery final
{
public:
    WorkoutQuery &whereId(int id);
    WorkoutQuery &whereName(const QString &name);
    WorkoutQuery &whereCreatedAfter(const QDateTime &dateTime);
    WorkoutQuery &whereCreatedBefore(const QDateTime &dateTime);
    WorkoutQuery &whereStartedTimeIsNull();
    WorkoutQuery &whereStartedTimeIsNotNull();
    WorkoutQuery &wherePlannedTimeIsNull();
    WorkoutQuery &wherePlannedTimeIsNotNull();
    WorkoutQuery &orderByCreatedTime(SortDirection direction);
    WorkoutQuery &orderByStartedTime(SortDirection direction);
    WorkoutQuery &orderByPlannedTime(SortDirection direction);
    WorkoutQuery &withLimit(int limit);
    WorkoutQuery &withOffset(int offset);

    const std::optional<int> &id() const;
    const std::optional<QString> &name() const;
    const std::optional<QDateTime> &createdAfter() const;
    const std::optional<QDateTime> &createdBefore() const;
    const std::optional<bool> &startedTimeIsNull() const;
    const std::optional<bool> &plannedTimeIsNull() const;
    const std::optional<SortDirection> &orderByCreatedTimeDirection() const;
    const std::optional<SortDirection> &orderByStartedTimeDirection() const;
    const std::optional<SortDirection> &orderByPlannedTimeDirection() const;
    const std::optional<int> &limit() const;
    const std::optional<int> &offset() const;

private:
    std::optional<int> m_id;
    std::optional<QString> m_name;
    std::optional<QDateTime> m_createdAfter;
    std::optional<QDateTime> m_createdBefore;
    std::optional<bool> m_startedTimeIsNull;
    std::optional<bool> m_plannedTimeIsNull;
    std::optional<SortDirection> m_orderByCreatedTimeDirection;
    std::optional<SortDirection> m_orderByStartedTimeDirection;
    std::optional<SortDirection> m_orderByPlannedTimeDirection;
    std::optional<int> m_limit;
    std::optional<int> m_offset;
};
