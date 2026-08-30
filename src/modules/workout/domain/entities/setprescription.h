#pragma once

#include "loadtype.h"
#include "set.h"
#include "setmetric.h"

class SetPrescription final
{
public:
    SetPrescription();
    SetPrescription(int repetitions, double weight);

    static SetPrescription createDuration(int seconds);
    static SetPrescription createDistance(double meters, int seconds);
    static SetPrescription fromSet(const Set& set);

    SetMetric metric() const;
    LoadType loadType() const;
    int repetitions() const;
    double weight() const;
    int durationSeconds() const;
    double distanceMeters() const;
    int restSecondsOverride() const;
    int position() const;

    void setMetric(SetMetric metric);
    void setLoadType(LoadType loadType);
    void setRepetitions(int repetitions);
    void setWeight(double weight);
    void setDurationSeconds(int seconds);
    void setDistanceMeters(double meters);
    void setRestSecondsOverride(int seconds);
    void setPosition(int position);

    Set toSet() const;

private:
    SetMetric m_metric { SetMetric::Reps };
    LoadType m_loadType { LoadType::External };
    int m_repetitions { 0 };
    double m_weight { 0.0 };
    int m_durationSeconds { 0 };
    double m_distanceMeters { 0.0 };
    int m_restSecondsOverride { -1 };
    int m_position { 0 };
};
