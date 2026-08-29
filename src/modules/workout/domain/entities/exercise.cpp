#include "exercise.h"
#include <QStringList>
#include <algorithm>

Exercise::Exercise() = default;

Exercise::Exercise(const QString& name, int restSeconds)
    : m_name(name)
    , m_restSeconds(restSeconds)
{
    validate();
}

int Exercise::id() const { return m_id; }
int Exercise::workoutId() const { return m_workoutId; }
const QString& Exercise::name() const { return m_name; }
const QString& Exercise::description() const { return m_description; }
int Exercise::restSeconds() const { return m_restSeconds; }
ExerciseKind Exercise::kind() const { return m_kind; }

void Exercise::setId(int id) { m_id = id; }
void Exercise::setWorkoutId(int workoutId) { m_workoutId = workoutId; }
void Exercise::setName(const QString& name) { m_name = name; }
void Exercise::setDescription(const QString& description) { m_description = description; }
void Exercise::setRestSeconds(int restSeconds) { m_restSeconds = restSeconds; }
void Exercise::setKind(ExerciseKind kind) { m_kind = kind; }

int Exercise::restSecondsForSet(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_sets.size()))
        return m_restSeconds;

    return m_sets[index].effectiveRestSeconds(m_restSeconds);
}

const std::vector<Set>& Exercise::sets() const { return m_sets; }
std::vector<Set>& Exercise::sets() { return m_sets; }

void Exercise::addSet(const Set& set) { m_sets.push_back(set); }

void Exercise::removeSet(int index)
{
    if (index >= 0 && index < static_cast<int>(m_sets.size()))
        m_sets.erase(m_sets.begin() + index);
}

bool Exercise::isCompleted() const
{
    if (m_sets.empty())
        return false;
    for (const auto& s : m_sets)
    {
        if (!s.completed())
            return false;
    }
    return true;
}

QString Exercise::setsToString() const
{
    QStringList parts;

    size_t i = 0;
    while (i < m_sets.size())
    {
        const QString compact = m_sets[i].toCompactString();
        const int restOverride = m_sets[i].restSecondsOverride();

        size_t runEnd = i + 1;
        while (runEnd < m_sets.size() && m_sets[runEnd].restSecondsOverride() == restOverride
               && m_sets[runEnd].toCompactString() == compact)
            ++runEnd;

        const int count = static_cast<int>(runEnd - i);

        if (restOverride >= 0)
        {
            parts.append(
                QString("%1x(%2/%3)").arg(count).arg(compact, Set::formatSeconds(restOverride)));
        }
        else
        {
            for (int n = 0; n < count; ++n)
                parts.append(compact);
        }

        i = runEnd;
    }

    return parts.join(", ");
}

double Exercise::totalWeight() const
{
    double total = 0.0;
    for (const auto& s : m_sets)
        total += s.totalWeight();
    return total;
}

int Exercise::totalRepetitions() const
{
    int total = 0;
    for (const auto& s : m_sets)
    {
        if (s.metric() == SetMetric::Reps)
            total += s.repetitions();
    }
    return total;
}

int Exercise::totalDurationSeconds() const
{
    int total = 0;
    for (const auto& s : m_sets)
        total += s.durationSeconds();
    return total;
}

double Exercise::totalDistanceMeters() const
{
    double total = 0.0;
    for (const auto& s : m_sets)
        total += s.distanceMeters();
    return total;
}

double Exercise::averageWeight() const
{
    double total = 0.0;
    int count = 0;
    for (const auto& s : m_sets)
    {
        if (!s.isWeighted())
            continue;
        total += s.weight();
        ++count;
    }
    return count > 0 ? total / count : 0.0;
}

double Exercise::bestOneRepMax() const
{
    double best = 0.0;
    for (const auto& s : m_sets)
        best = std::max(best, s.oneRepMax());
    return best;
}

void Exercise::validate() const { }
