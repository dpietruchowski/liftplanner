#pragma once

#include <optional>

class SetQuery final
{
public:
    SetQuery &whereId(int id);
    SetQuery &whereExerciseId(int exerciseId);
    SetQuery &withLimit(int limit);

    const std::optional<int> &id() const;
    const std::optional<int> &exerciseId() const;
    const std::optional<int> &limit() const;

private:
    std::optional<int> m_id;
    std::optional<int> m_exerciseId;
    std::optional<int> m_limit;
};
