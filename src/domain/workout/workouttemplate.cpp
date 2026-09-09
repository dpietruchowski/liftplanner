#include "workouttemplate.h"
#include "domain/ordered.h"
#include <algorithm>

WorkoutTemplate::WorkoutTemplate() = default;

WorkoutTemplate::WorkoutTemplate(const QString& name)
    : m_name(name)
{
}

int WorkoutTemplate::id() const { return m_id; }
const QString& WorkoutTemplate::name() const { return m_name; }
const QString& WorkoutTemplate::notes() const { return m_notes; }
const std::vector<TemplateExercise>& WorkoutTemplate::exercises() const { return m_exercises; }
std::vector<TemplateExercise>& WorkoutTemplate::exercises() { return m_exercises; }

void WorkoutTemplate::setId(int id) { m_id = id; }
void WorkoutTemplate::setName(const QString& name) { m_name = name; }
void WorkoutTemplate::setNotes(const QString& notes) { m_notes = notes; }

void WorkoutTemplate::addExercise(const TemplateExercise& exercise, int atPosition)
{
    Ordered::insert(m_exercises, exercise, atPosition);
}

void WorkoutTemplate::removeExercise(int index) { Ordered::remove(m_exercises, index); }

void WorkoutTemplate::moveExercise(int from, int to) { Ordered::move(m_exercises, from, to); }

void WorkoutTemplate::normalizePositions()
{
    Ordered::renumber(m_exercises);
    for (auto& exercise : m_exercises)
        exercise.normalizePositions();
}

bool WorkoutTemplate::isEmpty() const { return m_exercises.empty(); }

bool WorkoutTemplate::references(int definitionId) const
{
    return std::any_of(m_exercises.cbegin(), m_exercises.cend(),
                       [definitionId](const TemplateExercise& exercise)
                       { return exercise.definitionId() == definitionId; });
}

std::vector<int> WorkoutTemplate::definitionIds() const
{
    std::vector<int> ids;
    for (const auto& exercise : m_exercises)
    {
        if (std::find(ids.cbegin(), ids.cend(), exercise.definitionId()) == ids.cend())
            ids.push_back(exercise.definitionId());
    }
    return ids;
}

QStringList WorkoutTemplate::validationErrors() const
{
    QStringList errors;

    if (m_name.trimmed().isEmpty())
        errors.append(QStringLiteral("template name must not be empty"));

    if (m_exercises.empty())
        errors.append(QStringLiteral("template must contain at least one exercise"));

    for (size_t i = 0; i < m_exercises.size(); ++i)
    {
        for (const QString& error : m_exercises[i].validationErrors())
            errors.append(QStringLiteral("exercise %1: %2").arg(QString::number(i), error));
    }

    return errors;
}

bool WorkoutTemplate::isValid() const { return validationErrors().isEmpty(); }
