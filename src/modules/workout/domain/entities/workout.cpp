#include "workout.h"
#include "utils/timeprovider.h"

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
    m_startedTime = TimeProvider::instance().currentDateTime();
    m_status = WorkoutStatus::Started;
}

void Workout::end()
{
    m_endedTime = TimeProvider::instance().currentDateTime();
    m_status = WorkoutStatus::Ended;
}

const std::vector<Exercise>& Workout::exercises() const { return m_exercises; }
std::vector<Exercise>& Workout::exercises() { return m_exercises; }

void Workout::addExercise(const Exercise& exercise, int atPosition)
{
    const int size = static_cast<int>(m_exercises.size());
    const int index = (atPosition < 0 || atPosition > size) ? size : atPosition;

    m_exercises.insert(m_exercises.begin() + index, exercise);
    renumberExercises();
}

void Workout::removeExercise(int index)
{
    if (index < 0 || index >= static_cast<int>(m_exercises.size()))
        return;

    m_exercises.erase(m_exercises.begin() + index);
    renumberExercises();
}

void Workout::moveExercise(int from, int to)
{
    const int size = static_cast<int>(m_exercises.size());
    if (from < 0 || from >= size || to < 0 || to >= size || from == to)
        return;

    Exercise moved = m_exercises[from];
    m_exercises.erase(m_exercises.begin() + from);
    m_exercises.insert(m_exercises.begin() + to, moved);
    renumberExercises();
}

void Workout::normalizePositions()
{
    renumberExercises();
    for (auto& exercise : m_exercises)
        exercise.normalizePositions();
}

void Workout::renumberExercises()
{
    for (size_t i = 0; i < m_exercises.size(); ++i)
        m_exercises[i].setPosition(static_cast<int>(i));
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

bool Workout::isEmpty() const { return m_exercises.empty(); }

QStringList Workout::validationErrors() const
{
    QStringList errors;

    if (m_name.trimmed().isEmpty())
        errors.append(QStringLiteral("workout name must not be empty"));

    for (size_t i = 0; i < m_exercises.size(); ++i)
    {
        for (const QString& error : m_exercises[i].validationErrors())
            errors.append(QStringLiteral("exercise %1: %2").arg(QString::number(i), error));
    }

    return errors;
}

bool Workout::isValid() const { return validationErrors().isEmpty(); }

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

int Workout::totalDurationSeconds() const
{
    int total = 0;
    for (const auto& e : m_exercises)
        total += e.totalDurationSeconds();
    return total;
}

double Workout::totalDistanceMeters() const
{
    double total = 0.0;
    for (const auto& e : m_exercises)
        total += e.totalDistanceMeters();
    return total;
}

Workout Workout::createDefault(const QString& name)
{
    return Workout(name, TimeProvider::instance().currentDateTime());
}

void Workout::validate() const { }
