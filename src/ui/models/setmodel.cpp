#include "setmodel.h"

#include "domain/workout/performedsets.h"

SetModel::SetModel(QObject* parent)
    : QObject(parent)
{
}

SetModel::SetModel(const Set& set, QObject* parent)
    : QObject(parent)
    , m_set(set)
{
}

namespace
{

QString formatDuration(int seconds)
{
    if (seconds < 60)
        return QStringLiteral("%1 s").arg(seconds);
    return QStringLiteral("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QLatin1Char('0'));
}

}  // namespace

int SetModel::repetitions() const { return m_set.repetitions(); }
double SetModel::weight() const { return m_set.weight(); }
bool SetModel::completed() const { return m_set.completed(); }
int SetModel::durationSeconds() const { return m_set.durationSeconds(); }
double SetModel::distanceMeters() const { return m_set.distanceMeters(); }

QString SetModel::metric() const { return setMetricToString(m_set.metric()); }
QString SetModel::loadType() const { return loadTypeToString(m_set.loadType()); }

QString SetModel::primaryText() const
{
    switch (m_set.metric())
    {
        case SetMetric::Duration:
            return formatDuration(m_set.durationSeconds());

        case SetMetric::Distance:
            if (m_set.distanceMeters() >= 1000.0)
                return QStringLiteral("%1 km").arg(
                    QString::number(m_set.distanceMeters() / 1000.0, 'g', 4));
            return QStringLiteral("%1 m").arg(QString::number(m_set.distanceMeters(), 'g', 6));

        case SetMetric::Reps:
            break;
    }
    return QStringLiteral("%1 reps").arg(m_set.repetitions());
}

QString SetModel::secondaryText() const
{
    if (m_set.metric() == SetMetric::Duration)
        return QString();

    if (m_set.metric() == SetMetric::Distance)
        return m_set.durationSeconds() > 0 ? formatDuration(m_set.durationSeconds()) : QString();

    const QString weightText = QStringLiteral("%1 kg").arg(QString::number(m_set.weight(), 'g', 6));

    switch (m_set.loadType())
    {
        case LoadType::External:
            return weightText;
        case LoadType::Added:
            return QStringLiteral("BW + %1").arg(weightText);
        case LoadType::Assisted:
            return QStringLiteral("BW - %1").arg(weightText);
        case LoadType::Band:
            return QStringLiteral("band");
        case LoadType::Bodyweight:
        case LoadType::None:
            break;
    }
    return QStringLiteral("BW");
}

bool SetModel::secondaryAdjustable() const
{
    switch (m_set.metric())
    {
        case SetMetric::Duration:
            return false;
        case SetMetric::Distance:
            return true;
        case SetMetric::Reps:
            break;
    }

    return m_set.loadType() == LoadType::External || m_set.loadType() == LoadType::Added
        || m_set.loadType() == LoadType::Assisted;
}

void SetModel::setRepetitions(int value)
{
    if (m_set.repetitions() != value)
    {
        m_set.setRepetitions(value);
        emit repetitionsChanged();
        emit displayChanged();
    }
}

void SetModel::setWeight(double value)
{
    if (m_set.weight() != value)
    {
        m_set.setWeight(value);
        emit weightChanged();
        emit displayChanged();
    }
}

void SetModel::setCompleted(bool value)
{
    if (m_set.completed() != value)
    {
        m_set.setCompleted(value);
        emit completedChanged();
    }
}

void SetModel::setDurationSeconds(int value)
{
    if (m_set.durationSeconds() != value)
    {
        m_set.setDurationSeconds(value);
        emit durationSecondsChanged();
        emit displayChanged();
    }
}

void SetModel::setDistanceMeters(double value)
{
    if (m_set.distanceMeters() != value)
    {
        m_set.setDistanceMeters(value);
        emit distanceChanged();
        emit displayChanged();
    }
}

void SetModel::adoptPrescription(const Set& set)
{
    const Set previous = m_set;

    m_set = set;
    m_set.setId(previous.id());
    m_set.setExerciseId(previous.exerciseId());
    m_set.setCompleted(previous.completed());

    emit repetitionsChanged();
    emit weightChanged();
    emit durationSecondsChanged();
    emit distanceChanged();
    emit displayChanged();
}

bool SetModel::performed(bool completionFlagsMeaningful) const
{
    return countsAsPerformed(true, m_set.completed(), completionFlagsMeaningful);
}

const Set& SetModel::entity() const { return m_set; }
