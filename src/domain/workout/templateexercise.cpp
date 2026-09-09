#include "templateexercise.h"
#include "domain/ordered.h"
#include "domain/workout/restseconds.h"

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
    Ordered::insert(m_sets, prescription, atPosition);
}

void TemplateExercise::removeSet(int index) { Ordered::remove(m_sets, index); }

void TemplateExercise::moveSet(int from, int to) { Ordered::move(m_sets, from, to); }

void TemplateExercise::normalizePositions() { Ordered::renumber(m_sets); }

int TemplateExercise::effectiveRestSeconds(int definitionDefault) const
{
    return RestSeconds::effective(m_restSecondsOverride, definitionDefault);
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
