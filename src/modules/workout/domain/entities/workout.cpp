#include "workout.h"

Workout::Workout() = default;

Workout::Workout(const QString& name, const QDateTime& createdTime)
    : m_name(name)
    , m_createdTime(createdTime)
{
    validate();
}

int Workout::id() const { return m_id; }
const QString& Workout::name() const { return m_name; }
const QDateTime& Workout::createdTime() const { return m_createdTime; }
const QDateTime& Workout::plannedTime() const { return m_plannedTime; }
const QDateTime& Workout::startedTime() const { return m_startedTime; }
const QDateTime& Workout::endedTime() const { return m_endedTime; }

void Workout::setId(int id) { m_id = id; }
void Workout::setName(const QString& name) { m_name = name; }
void Workout::setCreatedTime(const QDateTime& createdTime) { m_createdTime = createdTime; }
void Workout::setPlannedTime(const QDateTime& plannedTime) { m_plannedTime = plannedTime; }
void Workout::setStartedTime(const QDateTime& startedTime) { m_startedTime = startedTime; }
void Workout::setEndedTime(const QDateTime& endedTime) { m_endedTime = endedTime; }

WorkoutStatus Workout::status() const { return m_status; }
void Workout::setStatus(WorkoutStatus status) { m_status = status; }

void Workout::start()
{
    m_startedTime = QDateTime::currentDateTime();
    m_status = WorkoutStatus::Started;
}

void Workout::end()
{
    m_endedTime = QDateTime::currentDateTime();
    m_status = WorkoutStatus::Ended;
}

const std::vector<Exercise>& Workout::exercises() const { return m_exercises; }
std::vector<Exercise>& Workout::exercises() { return m_exercises; }

void Workout::addExercise(const Exercise& exercise) { m_exercises.push_back(exercise); }

void Workout::removeExercise(int index)
{
    if (index >= 0 && index < static_cast<int>(m_exercises.size()))
        m_exercises.erase(m_exercises.begin() + index);
}

bool Workout::isCompleted() const
{
    if (m_exercises.empty())
        return false;
    for (const auto& e : m_exercises)
    {
        if (!e.isCompleted())
            return false;
    }
    return true;
}

int Workout::totalRepetitions() const
{
    int total = 0;
    for (const auto& e : m_exercises)
        total += e.totalRepetitions();
    return total;
}

int Workout::totalSets() const
{
    int total = 0;
    for (const auto& e : m_exercises)
        total += static_cast<int>(e.sets().size());
    return total;
}

double Workout::totalWeight() const
{
    double total = 0.0;
    for (const auto& e : m_exercises)
        total += e.totalWeight();
    return total;
}

Workout Workout::createDefault(const QString& name)
{
    return Workout(name, QDateTime::currentDateTime());
}

void Workout::validate() const { }
