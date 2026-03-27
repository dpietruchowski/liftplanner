#pragma once

#include <optional>
#include <QString>

class ExerciseQuery final
{
public:
    ExerciseQuery &whereId(int id);
    ExerciseQuery &whereWorkoutId(int workoutId);
    ExerciseQuery &whereName(const QString &name);
    ExerciseQuery &withLimit(int limit);

    const std::optional<int> &id() const;
    const std::optional<int> &workoutId() const;
    const std::optional<QString> &name() const;
    const std::optional<int> &limit() const;

private:
    std::optional<int> m_id;
    std::optional<int> m_workoutId;
    std::optional<QString> m_name;
    std::optional<int> m_limit;
};
