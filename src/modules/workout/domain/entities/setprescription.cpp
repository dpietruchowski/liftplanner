#include "setprescription.h"

SetPrescription::SetPrescription() = default;

SetPrescription::SetPrescription(int repetitions, double weight)
    : m_repetitions(repetitions)
    , m_weight(weight)
{
}

SetPrescription SetPrescription::createDuration(int seconds)
{
    SetPrescription prescription;
    prescription.m_metric = SetMetric::Duration;
    prescription.m_loadType = LoadType::None;
    prescription.m_durationSeconds = seconds;
    return prescription;
}

SetPrescription SetPrescription::createDistance(double meters, int seconds)
{
    SetPrescription prescription;
    prescription.m_metric = SetMetric::Distance;
    prescription.m_loadType = LoadType::None;
    prescription.m_distanceMeters = meters;
    prescription.m_durationSeconds = seconds;
    return prescription;
}

SetPrescription SetPrescription::fromSet(const Set& set)
{
    SetPrescription prescription;
    prescription.m_metric = set.metric();
    prescription.m_loadType = set.loadType();
    prescription.m_repetitions = set.repetitions();
    prescription.m_weight = set.weight();
    prescription.m_durationSeconds = set.durationSeconds();
    prescription.m_distanceMeters = set.distanceMeters();
    prescription.m_restSecondsOverride = set.restSecondsOverride();
    prescription.m_position = set.position();
    return prescription;
}

SetMetric SetPrescription::metric() const { return m_metric; }
LoadType SetPrescription::loadType() const { return m_loadType; }
int SetPrescription::repetitions() const { return m_repetitions; }
double SetPrescription::weight() const { return m_weight; }
int SetPrescription::durationSeconds() const { return m_durationSeconds; }
double SetPrescription::distanceMeters() const { return m_distanceMeters; }
int SetPrescription::restSecondsOverride() const { return m_restSecondsOverride; }
int SetPrescription::position() const { return m_position; }

void SetPrescription::setMetric(SetMetric metric) { m_metric = metric; }
void SetPrescription::setLoadType(LoadType loadType) { m_loadType = loadType; }
void SetPrescription::setRepetitions(int repetitions) { m_repetitions = repetitions; }
void SetPrescription::setWeight(double weight) { m_weight = weight; }
void SetPrescription::setDurationSeconds(int seconds) { m_durationSeconds = seconds; }
void SetPrescription::setDistanceMeters(double meters) { m_distanceMeters = meters; }
void SetPrescription::setRestSecondsOverride(int seconds) { m_restSecondsOverride = seconds; }
void SetPrescription::setPosition(int position) { m_position = position; }

Set SetPrescription::toSet() const
{
    Set set;
    set.setMetric(m_metric);
    set.setLoadType(m_loadType);
    set.setRepetitions(m_repetitions);
    set.setWeight(m_weight);
    set.setDurationSeconds(m_durationSeconds);
    set.setDistanceMeters(m_distanceMeters);
    set.setRestSecondsOverride(m_restSecondsOverride);
    set.setPosition(m_position);
    return set;
}
