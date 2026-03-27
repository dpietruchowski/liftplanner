#include "exercisemodel.h"

ExerciseModel::ExerciseModel(QObject *parent)
    : QObject(parent)
{
}

ExerciseModel::ExerciseModel(const Exercise &exercise, QObject *parent)
    : QObject(parent), m_exercise(exercise)
{
    for (const auto &set : exercise.sets())
    {
        auto *setModel = new SetModel(set, this);
        connect(setModel, &SetModel::completedChanged, this, &ExerciseModel::completedChanged);
        m_sets.append(setModel);
    }
}

QString ExerciseModel::name() const { return m_exercise.name(); }
QString ExerciseModel::description() const { return m_exercise.description(); }
int ExerciseModel::restSeconds() const { return m_exercise.restSeconds(); }
QString ExerciseModel::youtubeLink() const { return m_exercise.youtubeLink(); }

bool ExerciseModel::isCompleted() const
{
    if (m_sets.isEmpty())
        return false;
    for (auto *s : m_sets)
    {
        if (!s->completed())
            return false;
    }
    return true;
}

QQmlListProperty<SetModel> ExerciseModel::setsProperty()
{
    return QQmlListProperty<SetModel>(this, &m_sets);
}

QList<SetModel *> ExerciseModel::sets() const
{
    return m_sets;
}

QString ExerciseModel::setsToString() const
{
    QStringList parts;
    for (auto *s : m_sets)
        parts.append(QString("%1x%2kg").arg(s->repetitions()).arg(s->weight()));
    return parts.join(", ");
}

void ExerciseModel::addSet(SetModel *set)
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

void ExerciseModel::removeSet(SetModel *set)
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
    Exercise e;
    e.setId(m_exercise.id());
    e.setWorkoutId(m_exercise.workoutId());
    e.setName(m_exercise.name());
    e.setDescription(m_exercise.description());
    e.setRestSeconds(m_exercise.restSeconds());
    e.setYoutubeLink(m_exercise.youtubeLink());
    for (auto *s : m_sets)
        e.addSet(s->entity());
    return e;
}

ExerciseModel *ExerciseModel::clone(QObject *parent) const
{
    return new ExerciseModel(toEntity(), parent);
}
