#include "set.h"

Set::Set() = default;

Set::Set(int repetitions, double weight)
    : m_repetitions(repetitions)
    , m_weight(weight)
{
    validate();
}

int Set::id() const { return m_id; }
int Set::exerciseId() const { return m_exerciseId; }
int Set::repetitions() const { return m_repetitions; }
double Set::weight() const { return m_weight; }
bool Set::completed() const { return m_completed; }

void Set::setId(int id) { m_id = id; }
void Set::setExerciseId(int exerciseId) { m_exerciseId = exerciseId; }
void Set::setRepetitions(int repetitions) { m_repetitions = repetitions; }
void Set::setWeight(double weight) { m_weight = weight; }
void Set::setCompleted(bool completed) { m_completed = completed; }

double Set::totalWeight() const { return m_repetitions * m_weight; }

double Set::oneRepMax() const
{
    if (m_repetitions <= 0)
        return 0.0;
    if (m_repetitions == 1)
        return m_weight;
    // Brzycki for 2-10 reps, Epley above 10 (Brzycki loses accuracy at high reps).
    if (m_repetitions <= 10)
        return m_weight * 36.0 / (37 - m_repetitions);
    return m_weight * (1.0 + m_repetitions / 30.0);
}

void Set::validate() const { }
