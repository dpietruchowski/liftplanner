#include "workouttemplatemodel.h"

WorkoutTemplateModel::WorkoutTemplateModel(QObject* parent)
    : QObject(parent)
{
}

WorkoutTemplateModel::WorkoutTemplateModel(const WorkoutTemplateRow& row, QObject* parent)
    : QObject(parent)
    , m_row(row)
{
}

int WorkoutTemplateModel::templateId() const { return m_row.id; }

QString WorkoutTemplateModel::name() const { return m_row.name; }

QString WorkoutTemplateModel::notes() const { return m_row.notes; }

QStringList WorkoutTemplateModel::exerciseNames() const { return m_row.exerciseNames; }

int WorkoutTemplateModel::exerciseCount() const { return m_row.exerciseCount; }

int WorkoutTemplateModel::setCount() const { return m_row.setCount; }

bool WorkoutTemplateModel::isComplete() const { return m_row.complete; }

const WorkoutTemplateRow& WorkoutTemplateModel::entity() const { return m_row; }
