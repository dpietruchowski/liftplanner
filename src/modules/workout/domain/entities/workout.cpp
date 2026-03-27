#include "workout.h"

Workout::Workout() = default;

Workout::Workout(const QString &name, const QDateTime &createdTime)
    : m_name(name), m_createdTime(createdTime)
{
    validate();
}

int Workout::id() const { return m_id; }
const QString &Workout::name() const { return m_name; }
const QDateTime &Workout::createdTime() const { return m_createdTime; }
const QDateTime &Workout::startedTime() const { return m_startedTime; }
const QDateTime &Workout::endedTime() const { return m_endedTime; }

void Workout::setId(int id) { m_id = id; }
void Workout::setName(const QString &name) { m_name = name; }
void Workout::setCreatedTime(const QDateTime &createdTime) { m_createdTime = createdTime; }
void Workout::setStartedTime(const QDateTime &startedTime) { m_startedTime = startedTime; }
void Workout::setEndedTime(const QDateTime &endedTime) { m_endedTime = endedTime; }

const std::vector<Exercise> &Workout::exercises() const { return m_exercises; }
std::vector<Exercise> &Workout::exercises() { return m_exercises; }

void Workout::addExercise(const Exercise &exercise)
{
    m_exercises.push_back(exercise);
}

void Workout::removeExercise(int index)
{
    if (index >= 0 && index < static_cast<int>(m_exercises.size()))
        m_exercises.erase(m_exercises.begin() + index);
}

bool Workout::isCompleted() const
{
    if (m_exercises.empty())
        return false;
    for (const auto &e : m_exercises)
    {
        if (!e.isCompleted())
            return false;
    }
    return true;
}

Workout Workout::createDefault(const QString &name)
{
    return Workout(name, QDateTime::currentDateTime());
}

void Workout::validate() const
{
}
