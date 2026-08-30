#include "workouttemplate.h"
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
    const int size = static_cast<int>(m_exercises.size());
    const int index = (atPosition < 0 || atPosition > size) ? size : atPosition;

    m_exercises.insert(m_exercises.begin() + index, exercise);
    renumberExercises();
}

void WorkoutTemplate::removeExercise(int index)
{
    if (index < 0 || index >= static_cast<int>(m_exercises.size()))
        return;

    m_exercises.erase(m_exercises.begin() + index);
    renumberExercises();
}

void WorkoutTemplate::moveExercise(int from, int to)
{
    const int size = static_cast<int>(m_exercises.size());
    if (from < 0 || from >= size || to < 0 || to >= size || from == to)
        return;

    TemplateExercise moved = m_exercises[from];
    m_exercises.erase(m_exercises.begin() + from);
    m_exercises.insert(m_exercises.begin() + to, moved);
    renumberExercises();
}

void WorkoutTemplate::normalizePositions()
{
    renumberExercises();
    for (auto& exercise : m_exercises)
        exercise.normalizePositions();
}

void WorkoutTemplate::renumberExercises()
{
    for (size_t i = 0; i < m_exercises.size(); ++i)
        m_exercises[i].setPosition(static_cast<int>(i));
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
