#include "workouttimer.h"
#include "async/timeprovider.h"
#include <algorithm>

namespace
{
constexpr int tick_interval_ms = 250;
constexpr int minimum_adjusted_seconds = 1;
}  // namespace

WorkoutTimer::WorkoutTimer(QObject* parent)
    : QObject(parent)
{
    m_ticker.setInterval(tick_interval_ms);
    connect(&m_ticker, &QTimer::timeout, this, &WorkoutTimer::tick);
}

WorkoutTimer::Phase WorkoutTimer::phase() const { return m_phase; }
int WorkoutTimer::remainingSeconds() const { return m_remainingSeconds; }
int WorkoutTimer::totalSeconds() const { return m_totalSeconds; }
bool WorkoutTimer::isRunning() const { return m_phase != Idle; }
bool WorkoutTimer::isResting() const { return m_phase == Rest; }
bool WorkoutTimer::isPaused() const { return m_paused; }

QString WorkoutTimer::phaseLabel() const
{
    switch (m_phase)
    {
        case Work:
            return QStringLiteral("Work");
        case Rest:
            return QStringLiteral("Rest");
        case Idle:
            break;
    }
    return QString();
}

QString WorkoutTimer::remainingText() const
{
    return QStringLiteral("%1:%2")
        .arg(m_remainingSeconds / 60, 2, 10, QLatin1Char('0'))
        .arg(m_remainingSeconds % 60, 2, 10, QLatin1Char('0'));
}

void WorkoutTimer::start(Phase phase, int seconds)
{
    if (phase == Idle || seconds <= 0)
    {
        stop();
        return;
    }

    m_endTime = TimeProvider::instance().currentDateTime().addSecs(seconds);
    setPaused(false);
    setTotalSeconds(seconds);
    setRemainingSeconds(seconds);
    setPhase(phase);
    m_ticker.start();
}

void WorkoutTimer::stop()
{
    if (m_phase == Idle && !m_paused && m_remainingSeconds == 0)
        return;

    reset();
}

void WorkoutTimer::pause()
{
    if (m_phase == Idle || m_paused)
        return;

    tick();
    if (m_phase == Idle)
        return;

    m_ticker.stop();
    setPaused(true);
}

void WorkoutTimer::resume()
{
    if (m_phase == Idle || !m_paused)
        return;

    m_endTime = TimeProvider::instance().currentDateTime().addSecs(m_remainingSeconds);
    setPaused(false);
    m_ticker.start();
}

void WorkoutTimer::addSeconds(int seconds)
{
    if (m_phase == Idle || seconds == 0)
        return;

    const int adjusted = std::max(minimum_adjusted_seconds, m_remainingSeconds + seconds);
    if (!m_paused)
        m_endTime = TimeProvider::instance().currentDateTime().addSecs(adjusted);

    setTotalSeconds(std::max(m_totalSeconds, adjusted));
    setRemainingSeconds(adjusted);
}

void WorkoutTimer::tick()
{
    if (m_phase == Idle || m_paused || !m_endTime.isValid())
        return;

    const qint64 msecs = TimeProvider::instance().currentDateTime().msecsTo(m_endTime);
    if (msecs > 0)
    {
        setRemainingSeconds(static_cast<int>((msecs + 999) / 1000));
        return;
    }

    const Phase finishedPhase = m_phase;
    reset();
    emit finished(finishedPhase);
}

void WorkoutTimer::setRemainingSeconds(int seconds)
{
    if (m_remainingSeconds == seconds)
        return;

    m_remainingSeconds = seconds;
    emit remainingSecondsChanged();
}

void WorkoutTimer::setTotalSeconds(int seconds)
{
    if (m_totalSeconds == seconds)
        return;

    m_totalSeconds = seconds;
    emit totalSecondsChanged();
}

void WorkoutTimer::setPhase(Phase phase)
{
    if (m_phase == phase)
        return;

    m_phase = phase;
    emit phaseChanged();
}

void WorkoutTimer::setPaused(bool paused)
{
    if (m_paused == paused)
        return;

    m_paused = paused;
    emit pausedChanged();
}

void WorkoutTimer::reset()
{
    m_ticker.stop();
    m_endTime = QDateTime();
    setPaused(false);
    setTotalSeconds(0);
    setRemainingSeconds(0);
    setPhase(Idle);
}
