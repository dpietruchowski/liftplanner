#include "exercise.h"
#include <QStringList>

Exercise::Exercise() = default;

Exercise::Exercise(const QString &name, int restSeconds)
    : m_name(name), m_restSeconds(restSeconds)
{
    validate();
}

int Exercise::id() const { return m_id; }
int Exercise::workoutId() const { return m_workoutId; }
const QString &Exercise::name() const { return m_name; }
const QString &Exercise::description() const { return m_description; }
int Exercise::restSeconds() const { return m_restSeconds; }
const QString &Exercise::youtubeLink() const { return m_youtubeLink; }

void Exercise::setId(int id) { m_id = id; }
void Exercise::setWorkoutId(int workoutId) { m_workoutId = workoutId; }
void Exercise::setName(const QString &name) { m_name = name; }
void Exercise::setDescription(const QString &description) { m_description = description; }
void Exercise::setRestSeconds(int restSeconds) { m_restSeconds = restSeconds; }
void Exercise::setYoutubeLink(const QString &youtubeLink) { m_youtubeLink = youtubeLink; }

const std::vector<Set> &Exercise::sets() const { return m_sets; }
std::vector<Set> &Exercise::sets() { return m_sets; }

void Exercise::addSet(const Set &set)
{
    m_sets.push_back(set);
}

void Exercise::removeSet(int index)
{
    if (index >= 0 && index < static_cast<int>(m_sets.size()))
        m_sets.erase(m_sets.begin() + index);
}

bool Exercise::isCompleted() const
{
    if (m_sets.empty())
        return false;
    for (const auto &s : m_sets)
    {
        if (!s.completed())
            return false;
    }
    return true;
}

QString Exercise::setsToString() const
{
    QStringList parts;
    for (const auto &s : m_sets)
        parts.append(QString("%1x%2kg").arg(s.repetitions()).arg(s.weight()));
    return parts.join(", ");
}

void Exercise::validate() const
{
}
