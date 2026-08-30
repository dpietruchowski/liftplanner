#include "templateexercise.h"

TemplateExercise::TemplateExercise() = default;

TemplateExercise::TemplateExercise(int definitionId)
    : m_definitionId(definitionId)
{
}

int TemplateExercise::definitionId() const { return m_definitionId; }
int TemplateExercise::position() const { return m_position; }
int TemplateExercise::restSecondsOverride() const { return m_restSecondsOverride; }
const QString& TemplateExercise::notes() const { return m_notes; }
const std::vector<SetPrescription>& TemplateExercise::sets() const { return m_sets; }
std::vector<SetPrescription>& TemplateExercise::sets() { return m_sets; }

void TemplateExercise::setDefinitionId(int definitionId) { m_definitionId = definitionId; }
void TemplateExercise::setPosition(int position) { m_position = position; }
void TemplateExercise::setRestSecondsOverride(int seconds) { m_restSecondsOverride = seconds; }
void TemplateExercise::setNotes(const QString& notes) { m_notes = notes; }

void TemplateExercise::addSet(const SetPrescription& prescription, int atPosition)
{
    const int size = static_cast<int>(m_sets.size());
    const int index = (atPosition < 0 || atPosition > size) ? size : atPosition;

    m_sets.insert(m_sets.begin() + index, prescription);
    renumberSets();
}

void TemplateExercise::removeSet(int index)
{
    if (index < 0 || index >= static_cast<int>(m_sets.size()))
        return;

    m_sets.erase(m_sets.begin() + index);
    renumberSets();
}

void TemplateExercise::moveSet(int from, int to)
{
    const int size = static_cast<int>(m_sets.size());
    if (from < 0 || from >= size || to < 0 || to >= size || from == to)
        return;

    SetPrescription moved = m_sets[from];
    m_sets.erase(m_sets.begin() + from);
    m_sets.insert(m_sets.begin() + to, moved);
    renumberSets();
}

void TemplateExercise::normalizePositions() { renumberSets(); }

void TemplateExercise::renumberSets()
{
    for (size_t i = 0; i < m_sets.size(); ++i)
        m_sets[i].setPosition(static_cast<int>(i));
}

int TemplateExercise::effectiveRestSeconds(int definitionDefault) const
{
    return m_restSecondsOverride >= 0 ? m_restSecondsOverride : definitionDefault;
}

Exercise TemplateExercise::toExercise(const QString& name, ExerciseKind kind,
                                      int definitionRestSeconds) const
{
    Exercise exercise = Exercise::createFromDefinition(m_definitionId, name, kind,
                                                       effectiveRestSeconds(definitionRestSeconds));
    exercise.setNotes(m_notes);

    for (const auto& prescription : m_sets)
        exercise.addSet(prescription.toSet());

    return exercise;
}

QStringList TemplateExercise::validationErrors() const
{
    QStringList errors;

    if (m_definitionId < 0)
        errors.append(QStringLiteral("template exercise must reference a catalog definition"));

    if (m_sets.empty())
        errors.append(QStringLiteral("template exercise must prescribe at least one set"));

    return errors;
}

bool TemplateExercise::isValid() const { return validationErrors().isEmpty(); }
