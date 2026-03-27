#include "setmodel.h"

SetModel::SetModel(QObject *parent)
    : QObject(parent)
{
}

SetModel::SetModel(const Set &set, QObject *parent)
    : QObject(parent), m_set(set)
{
}

int SetModel::repetitions() const { return m_set.repetitions(); }
int SetModel::weight() const { return m_set.weight(); }
bool SetModel::completed() const { return m_set.completed(); }

void SetModel::setRepetitions(int value)
{
    if (m_set.repetitions() != value)
    {
        m_set.setRepetitions(value);
        emit repetitionsChanged();
    }
}

void SetModel::setWeight(int value)
{
    if (m_set.weight() != value)
    {
        m_set.setWeight(value);
        emit weightChanged();
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

const Set &SetModel::entity() const { return m_set; }

SetModel *SetModel::clone(QObject *parent) const
{
    return new SetModel(m_set, parent);
}
