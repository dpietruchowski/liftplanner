#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

class WorkoutTimer : public QObject
{
    Q_OBJECT

public:
    enum Phase
    {
        Idle,
        Work,
        Rest
    };
    Q_ENUM(Phase)

    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY remainingSecondsChanged)
    Q_PROPERTY(int totalSeconds READ totalSeconds NOTIFY totalSecondsChanged)
    Q_PROPERTY(QString phaseLabel READ phaseLabel NOTIFY phaseChanged)
    Q_PROPERTY(QString remainingText READ remainingText NOTIFY remainingSecondsChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY phaseChanged)
    Q_PROPERTY(bool resting READ isResting NOTIFY phaseChanged)
    Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)

    explicit WorkoutTimer(QObject* parent = nullptr);

    Phase phase() const;
    int remainingSeconds() const;
    int totalSeconds() const;
    QString phaseLabel() const;
    QString remainingText() const;
    bool isRunning() const;
    bool isResting() const;
    bool isPaused() const;

    void start(Phase phase, int seconds);

    Q_INVOKABLE void stop();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void addSeconds(int seconds);
    Q_INVOKABLE void tick();

signals:
    void remainingSecondsChanged();
    void totalSecondsChanged();
    void phaseChanged();
    void pausedChanged();
    void finished(Phase phase);

private:
    void setRemainingSeconds(int seconds);
    void setTotalSeconds(int seconds);
    void setPhase(Phase phase);
    void setPaused(bool paused);
    void reset();

    QTimer m_ticker;
    QDateTime m_endTime;
    Phase m_phase { Idle };
    int m_totalSeconds { 0 };
    int m_remainingSeconds { 0 };
    bool m_paused { false };
};
