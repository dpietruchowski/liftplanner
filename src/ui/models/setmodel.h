#pragma once

#include <QObject>
#include "modules/workout/domain/entities/set.h"

class SetModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int repetitions READ repetitions WRITE setRepetitions NOTIFY repetitionsChanged)
    Q_PROPERTY(int weight READ weight WRITE setWeight NOTIFY weightChanged)
    Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged)

public:
    explicit SetModel(QObject *parent = nullptr);
    explicit SetModel(const Set &set, QObject *parent = nullptr);

    int repetitions() const;
    int weight() const;
    bool completed() const;

    void setRepetitions(int value);
    void setWeight(int value);
    void setCompleted(bool value);

    const Set &entity() const;
    SetModel *clone(QObject *parent = nullptr) const;

signals:
    void repetitionsChanged();
    void weightChanged();
    void completedChanged();

private:
    Set m_set;
};
