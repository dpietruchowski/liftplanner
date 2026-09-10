#include "exercisemodel.h"

#include "ui/presentation/workouttext.h"

namespace
{
Exercise recordOf(const Exercise& exercise)
{
    Exercise record = exercise;
    record.sets().clear();
    return record;
}
}

ExerciseModel::ExerciseModel(QObject* parent)
    : QObject(parent)
{
}

ExerciseModel::ExerciseModel(const Exercise& exercise, QObject* parent)
    : QObject(parent)
    , m_record(recordOf(exercise))
{
    for (const auto& set : exercise.sets())
    {
        auto* setModel = new SetModel(set, this);
        connect(setModel, &SetModel::completedChanged, this, &ExerciseModel::completedChanged);
        m_sets.append(setModel);
    }
}

QString ExerciseModel::name() const { return m_record.name(); }
QString ExerciseModel::description() const { return m_record.description(); }
int ExerciseModel::restSeconds() const { return m_record.restSeconds(); }
QString ExerciseModel::restText() const { return WorkoutText::formatRest(m_record.restSeconds()); }
QString ExerciseModel::kindString() const { return exerciseKindToString(m_record.kind()); }

bool ExerciseModel::isCompleted() const { return toEntity().isCompleted(); }

QString ExerciseModel::previousSummary() const { return m_previousSummary; }
QDateTime ExerciseModel::previousDate() const { return m_previousDate; }

void ExerciseModel::setPreviousPerformance(const QString& summary, const QDateTime& date)
{
    if (m_previousSummary == summary && m_previousDate == date)
        return;

    m_previousSummary = summary;
    m_previousDate = date;
    emit previousPerformanceChanged();
}

QQmlListProperty<SetModel> ExerciseModel::setsProperty()
{
    return QQmlListProperty<SetModel>(this, &m_sets);
}

QList<SetModel*> ExerciseModel::sets() const { return m_sets; }

QString ExerciseModel::setsToString() const { return toEntity().setsToString(); }

void ExerciseModel::addSet(SetModel* set)
{
    if (set && !m_sets.contains(set))
    {
        set->setParent(this);
        connect(set, &SetModel::completedChanged, this, &ExerciseModel::completedChanged);
        m_sets.append(set);
        emit setsChanged();
        emit completedChanged();
    }
}

void ExerciseModel::removeSet(SetModel* set)
{
    if (set && m_sets.contains(set))
    {
        m_sets.removeAll(set);
        set->setParent(nullptr);
        emit setsChanged();
        emit completedChanged();
    }
}

Exercise ExerciseModel::toEntity() const
{
    Exercise exercise = m_record;
    for (auto* set : m_sets)
        exercise.addSet(set->entity());
    return exercise;
}
