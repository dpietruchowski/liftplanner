#include "workout.h"
#include "async/timeprovider.h"
#include "domain/ordered.h"
#include "domain/workout/generatedworkoutname.h"

Workout::Workout() = default;

Workout::Workout(const QString& name, const QDateTime& createdTime)
    : m_name(name)
    , m_createdTime(createdTime)
{
}

int Workout::id() const { return m_id; }
const QString& Workout::name() const { return m_name; }
const QDateTime& Workout::createdTime() const { return m_createdTime; }
const QDateTime& Workout::plannedTime() const { return m_plannedTime; }
const QDateTime& Workout::startedTime() const { return m_startedTime; }
const QDateTime& Workout::endedTime() const { return m_endedTime; }

bool Workout::hasGeneratedName() const { return m_generatedName; }

void Workout::setId(int id) { m_id = id; }
void Workout::setName(const QString& name) { m_name = name; }
void Workout::setGeneratedName(bool generated) { m_generatedName = generated; }
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

    if (m_generatedName)
        m_name = generatedSessionName(m_startedTime);
}

void Workout::end()
{
    m_endedTime = TimeProvider::instance().currentDateTime();
    m_status = WorkoutStatus::Ended;
}

void Workout::returnToPlan()
{
    m_status = WorkoutStatus::Planned;

    if (m_generatedName)
        m_name = generatedPlanName();
}

const std::vector<Exercise>& Workout::exercises() const { return m_exercises; }
std::vector<Exercise>& Workout::exercises() { return m_exercises; }

void Workout::addExercise(const Exercise& exercise, int atPosition)
{
    Ordered::insert(m_exercises, exercise, atPosition);
}

void Workout::removeExercise(int index) { Ordered::remove(m_exercises, index); }

void Workout::moveExercise(int from, int to) { Ordered::move(m_exercises, from, to); }

void Workout::normalizePositions()
{
    Ordered::renumber(m_exercises);
    for (auto& exercise : m_exercises)
        exercise.normalizePositions();
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
