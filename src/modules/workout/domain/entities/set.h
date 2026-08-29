#pragma once

#include "loadtype.h"
#include "setmetric.h"
#include <QString>

class Set final
{
public:
    Set();
    Set(int repetitions, double weight);

    static Set createDuration(int seconds);
    static Set createDistance(double meters, int seconds);

    int id() const;
    int exerciseId() const;
    int repetitions() const;
    double weight() const;
    bool completed() const;
    SetMetric metric() const;
    LoadType loadType() const;
    int durationSeconds() const;
    double distanceMeters() const;
    int restSecondsOverride() const;

    bool isWeighted() const;
    int effectiveRestSeconds(int exerciseDefault) const;
    QString toCompactString() const;

    static QString formatSeconds(int seconds);

    double totalWeight() const;
    double oneRepMax() const;

    void setId(int id);
    void setExerciseId(int exerciseId);
    void setRepetitions(int repetitions);
    void setWeight(double weight);
    void setCompleted(bool completed);
    void setMetric(SetMetric metric);
    void setLoadType(LoadType loadType);
    void setDurationSeconds(int seconds);
    void setDistanceMeters(double meters);
    void setRestSecondsOverride(int seconds);

private:
    void validate() const;

    int m_id { -1 };
    int m_exerciseId { -1 };
    int m_repetitions { 0 };
    double m_weight { 0.0 };
    bool m_completed { false };
    SetMetric m_metric { SetMetric::Reps };
    LoadType m_loadType { LoadType::External };
    int m_durationSeconds { 0 };
    double m_distanceMeters { 0.0 };
    int m_restSecondsOverride { -1 };
};
