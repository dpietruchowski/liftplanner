#include "exercise.h"
#include "setcompatibility.h"
#include <QStringList>
#include <algorithm>

Exercise::Exercise() = default;

Exercise::Exercise(const QString& name, int restSeconds)
    : m_name(name)
    , m_restSeconds(restSeconds)
{
}

Exercise Exercise::createFromDefinition(int definitionId, const QString& name, ExerciseKind kind,
                                        int restSeconds)
{
    Exercise exercise(name, restSeconds);
    exercise.m_definitionId = definitionId;
    exercise.m_kind = kind;
    return exercise;
}

Exercise Exercise::createAdHoc(const QString& name, ExerciseKind kind, int restSeconds)
{
    Exercise exercise(name, restSeconds);
    exercise.m_kind = kind;
    return exercise;
}

int Exercise::id() const { return m_id; }
int Exercise::workoutId() const { return m_workoutId; }
const QString& Exercise::name() const { return m_name; }
const QString& Exercise::description() const { return m_description; }
int Exercise::restSeconds() const { return m_restSeconds; }
ExerciseKind Exercise::kind() const { return m_kind; }
int Exercise::position() const { return m_position; }
const std::optional<int>& Exercise::definitionId() const { return m_definitionId; }
bool Exercise::hasDefinition() const { return m_definitionId.has_value(); }
const QString& Exercise::notes() const { return m_notes; }

void Exercise::setId(int id) { m_id = id; }
void Exercise::setWorkoutId(int workoutId) { m_workoutId = workoutId; }
void Exercise::setName(const QString& name) { m_name = name; }
void Exercise::setDescription(const QString& description) { m_description = description; }
void Exercise::setRestSeconds(int restSeconds) { m_restSeconds = restSeconds; }
void Exercise::setKind(ExerciseKind kind) { m_kind = kind; }
void Exercise::setPosition(int position) { m_position = position; }
void Exercise::setDefinitionId(int definitionId) { m_definitionId = definitionId; }
void Exercise::clearDefinitionId() { m_definitionId.reset(); }
void Exercise::setNotes(const QString& notes) { m_notes = notes; }

int Exercise::restSecondsForSet(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_sets.size()))
        return m_restSeconds;

    return m_sets[index].effectiveRestSeconds(m_restSeconds);
}

const std::vector<Set>& Exercise::sets() const { return m_sets; }
std::vector<Set>& Exercise::sets() { return m_sets; }

void Exercise::addSet(const Set& set, int atPosition)
{
    const int size = static_cast<int>(m_sets.size());
    const int index = (atPosition < 0 || atPosition > size) ? size : atPosition;

    m_sets.insert(m_sets.begin() + index, set);
    renumberSets();
}

void Exercise::removeSet(int index)
{
    if (index < 0 || index >= static_cast<int>(m_sets.size()))
        return;

    m_sets.erase(m_sets.begin() + index);
    renumberSets();
}

void Exercise::moveSet(int from, int to)
{
    const int size = static_cast<int>(m_sets.size());
    if (from < 0 || from >= size || to < 0 || to >= size || from == to)
        return;

    Set moved = m_sets[from];
    m_sets.erase(m_sets.begin() + from);
    m_sets.insert(m_sets.begin() + to, moved);
    renumberSets();
}

void Exercise::duplicateSet(int index)
{
    if (index < 0 || index >= static_cast<int>(m_sets.size()))
        return;

    Set copy = m_sets[index];
    copy.setId(-1);
    copy.setCompleted(false);

    m_sets.insert(m_sets.begin() + index + 1, copy);
    renumberSets();
}

void Exercise::normalizePositions() { renumberSets(); }

void Exercise::renumberSets()
{
    for (size_t i = 0; i < m_sets.size(); ++i)
        m_sets[i].setPosition(static_cast<int>(i));
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

bool Exercise::isWeighted() const
{
    return std::any_of(m_sets.cbegin(), m_sets.cend(),
                       [](const Set& set) { return set.isWeighted(); });
}

double Exercise::bestOneRepMax() const
{
    double best = 0.0;
    for (const auto& s : m_sets)
        best = std::max(best, s.oneRepMax());
    return best;
}

bool Exercise::acceptsSet(const Set& set) const { return metricSuitsKind(m_kind, set.metric()); }

QStringList Exercise::validationErrors() const
{
    QStringList errors;

    if (m_name.trimmed().isEmpty())
        errors.append(QStringLiteral("exercise name must not be empty"));

    if (m_restSeconds < 0)
        errors.append(QStringLiteral("rest seconds must not be negative"));

    for (size_t i = 0; i < m_sets.size(); ++i)
    {
        if (acceptsSet(m_sets[i]))
            continue;

        errors.append(QStringLiteral("set %1 uses metric %2 which does not suit kind %3")
                          .arg(QString::number(i), setMetricToString(m_sets[i].metric()),
                               exerciseKindToString(m_kind)));
    }

    return errors;
}

bool Exercise::isValid() const { return validationErrors().isEmpty(); }
