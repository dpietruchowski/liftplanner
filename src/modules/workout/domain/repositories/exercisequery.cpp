#include "exercisequery.h"

#include <QString>

ExerciseQuery &ExerciseQuery::whereId(int id)
{
    m_id = id;
    return *this;
}
ExerciseQuery &ExerciseQuery::whereWorkoutId(int workoutId)
{
    m_workoutId = workoutId;
    return *this;
}
ExerciseQuery &ExerciseQuery::whereName(const QString &name)
{
    m_name = name;
    return *this;
}
ExerciseQuery &ExerciseQuery::withLimit(int limit)
{
    m_limit = limit;
    return *this;
}

const std::optional<int> &ExerciseQuery::id() const { return m_id; }
const std::optional<int> &ExerciseQuery::workoutId() const { return m_workoutId; }
const std::optional<QString> &ExerciseQuery::name() const { return m_name; }
const std::optional<int> &ExerciseQuery::limit() const { return m_limit; }
