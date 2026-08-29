#include "set.h"
#include <cmath>

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

QString Set::formatSeconds(int seconds)
{
    if (seconds >= 60 && seconds % 60 == 0)
        return QString::number(seconds / 60) + QStringLiteral("min");
    return QString::number(seconds) + QStringLiteral("s");
}

QString Set::toCompactString() const
{
    const QString weightText = QString::number(m_weight, 'g', 6);

    switch (m_metric)
    {
        case SetMetric::Duration:
            return formatSeconds(m_durationSeconds);

        case SetMetric::Distance:
        {
            QString distanceText;
            if (m_distanceMeters >= 1000.0 && std::fmod(m_distanceMeters, 1000.0) == 0.0)
                distanceText
                    = QString::number(m_distanceMeters / 1000.0, 'g', 6) + QStringLiteral("km");
            else
                distanceText = QString::number(m_distanceMeters, 'g', 6) + QStringLiteral("m");

            if (m_durationSeconds > 0)
                distanceText += QStringLiteral("@") + formatSeconds(m_durationSeconds);
            return distanceText;
        }

        case SetMetric::Reps:
            break;
    }

    const QString repsText = QString::number(m_repetitions) + QStringLiteral("x");

    switch (m_loadType)
    {
        case LoadType::External:
            return repsText + weightText + QStringLiteral("kg");
        case LoadType::Added:
            return repsText + QStringLiteral("BW+") + weightText + QStringLiteral("kg");
        case LoadType::Assisted:
            return repsText + QStringLiteral("BW-") + weightText + QStringLiteral("kg");
        case LoadType::Band:
            return repsText + QStringLiteral("BAND");
        case LoadType::Bodyweight:
        case LoadType::None:
            break;
    }
    return repsText + QStringLiteral("BW");
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
