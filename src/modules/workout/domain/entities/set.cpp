#include "set.h"

Set::Set() = default;

Set::Set(int repetitions, int weight)
    : m_repetitions(repetitions), m_weight(weight)
{
    validate();
}

int Set::id() const { return m_id; }
int Set::exerciseId() const { return m_exerciseId; }
int Set::repetitions() const { return m_repetitions; }
int Set::weight() const { return m_weight; }
bool Set::completed() const { return m_completed; }

void Set::setId(int id) { m_id = id; }
void Set::setExerciseId(int exerciseId) { m_exerciseId = exerciseId; }
void Set::setRepetitions(int repetitions) { m_repetitions = repetitions; }
void Set::setWeight(int weight) { m_weight = weight; }
void Set::setCompleted(bool completed) { m_completed = completed; }

void Set::validate() const
{
}
