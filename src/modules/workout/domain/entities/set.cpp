#include "set.h"

Set::Set() = default;

Set::Set(int repetitions, double weight)
    : m_repetitions(repetitions)
    , m_weight(weight)
{
    validate();
}

Set Set::createDuration(int seconds)
{
    Set set;
    set.m_metric = SetMetric::Duration;
    set.m_loadType = LoadType::None;
    set.m_durationSeconds = seconds;
    set.validate();
    return set;
}

Set Set::createDistance(double meters, int seconds)
{
    Set set;
    set.m_metric = SetMetric::Distance;
    set.m_loadType = LoadType::None;
    set.m_distanceMeters = meters;
    set.m_durationSeconds = seconds;
    set.validate();
    return set;
}

int Set::id() const { return m_id; }
int Set::exerciseId() const { return m_exerciseId; }
int Set::repetitions() const { return m_repetitions; }
double Set::weight() const { return m_weight; }
bool Set::completed() const { return m_completed; }
SetMetric Set::metric() const { return m_metric; }
LoadType Set::loadType() const { return m_loadType; }
int Set::durationSeconds() const { return m_durationSeconds; }
double Set::distanceMeters() const { return m_distanceMeters; }
int Set::restSecondsOverride() const { return m_restSecondsOverride; }

void Set::setId(int id) { m_id = id; }
void Set::setExerciseId(int exerciseId) { m_exerciseId = exerciseId; }
void Set::setRepetitions(int repetitions) { m_repetitions = repetitions; }
void Set::setWeight(double weight) { m_weight = weight; }
void Set::setCompleted(bool completed) { m_completed = completed; }
void Set::setMetric(SetMetric metric) { m_metric = metric; }
void Set::setLoadType(LoadType loadType) { m_loadType = loadType; }
void Set::setDurationSeconds(int seconds) { m_durationSeconds = seconds; }
void Set::setDistanceMeters(double meters) { m_distanceMeters = meters; }
void Set::setRestSecondsOverride(int seconds) { m_restSecondsOverride = seconds; }

bool Set::isWeighted() const
{
    return m_metric == SetMetric::Reps && m_loadType == LoadType::External;
}

double Set::totalWeight() const
{
    if (!isWeighted())
        return 0.0;
    return m_repetitions * m_weight;
}

double Set::oneRepMax() const
{
    if (!isWeighted())
        return 0.0;
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
