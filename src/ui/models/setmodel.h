#pragma once

#include "modules/workout/domain/entities/set.h"
#include <QObject>

class SetModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int repetitions READ repetitions WRITE setRepetitions NOTIFY repetitionsChanged)
    Q_PROPERTY(double weight READ weight WRITE setWeight NOTIFY weightChanged)
    Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged)

public:
    explicit SetModel(QObject* parent = nullptr);
    explicit SetModel(const Set& set, QObject* parent = nullptr);

    int repetitions() const;
    double weight() const;
    bool completed() const;

    void setRepetitions(int value);
    void setWeight(double value);
    void setCompleted(bool value);

    const Set& entity() const;

signals:
    void repetitionsChanged();
    void weightChanged();
    void completedChanged();

private:
    Set m_set;
};
