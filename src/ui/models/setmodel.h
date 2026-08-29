#pragma once

#include "modules/workout/domain/entities/set.h"
#include <QObject>
#include <QString>

class SetModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int repetitions READ repetitions WRITE setRepetitions NOTIFY repetitionsChanged)
    Q_PROPERTY(double weight READ weight WRITE setWeight NOTIFY weightChanged)
    Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged)
    Q_PROPERTY(int durationSeconds READ durationSeconds WRITE setDurationSeconds NOTIFY
                   durationSecondsChanged)
    Q_PROPERTY(
        double distanceMeters READ distanceMeters WRITE setDistanceMeters NOTIFY distanceChanged)
    Q_PROPERTY(QString metric READ metric NOTIFY displayChanged)
    Q_PROPERTY(QString loadType READ loadType NOTIFY displayChanged)
    Q_PROPERTY(QString primaryText READ primaryText NOTIFY displayChanged)
    Q_PROPERTY(QString secondaryText READ secondaryText NOTIFY displayChanged)
    Q_PROPERTY(bool secondaryAdjustable READ secondaryAdjustable NOTIFY displayChanged)

public:
    explicit SetModel(QObject* parent = nullptr);
    explicit SetModel(const Set& set, QObject* parent = nullptr);

    int repetitions() const;
    double weight() const;
    bool completed() const;
    int durationSeconds() const;
    double distanceMeters() const;

    QString metric() const;
    QString loadType() const;
    QString primaryText() const;
    QString secondaryText() const;
    bool secondaryAdjustable() const;

    void setRepetitions(int value);
    void setWeight(double value);
    void setCompleted(bool value);
    void setDurationSeconds(int value);
    void setDistanceMeters(double value);

    const Set& entity() const;

signals:
    void repetitionsChanged();
    void weightChanged();
    void completedChanged();
    void durationSecondsChanged();
    void distanceChanged();
    void displayChanged();

private:
    Set m_set;
};
