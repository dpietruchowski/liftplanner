#include "workouttemplatemodel.h"

WorkoutTemplateModel::WorkoutTemplateModel(QObject* parent)
    : QObject(parent)
{
}

WorkoutTemplateModel::WorkoutTemplateModel(const WorkoutTemplateSummary& summary, QObject* parent)
    : QObject(parent)
    , m_summary(summary)
{
}

int WorkoutTemplateModel::templateId() const { return m_summary.workoutTemplate.id(); }

QString WorkoutTemplateModel::name() const { return m_summary.workoutTemplate.name(); }

QString WorkoutTemplateModel::notes() const { return m_summary.workoutTemplate.notes(); }

QStringList WorkoutTemplateModel::exerciseNames() const { return m_summary.exerciseNames; }

int WorkoutTemplateModel::exerciseCount() const
{
    return static_cast<int>(m_summary.workoutTemplate.exercises().size());
}

int WorkoutTemplateModel::setCount() const { return m_summary.setCount; }

bool WorkoutTemplateModel::isComplete() const { return m_summary.complete; }

const WorkoutTemplateSummary& WorkoutTemplateModel::entity() const { return m_summary; }
