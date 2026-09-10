#include "ui/presentation/workoutstartpolicy.h"

#include "ui/models/workoutmodel.h"
#include <QDateTime>
#include <QLocale>

namespace
{

QVariantMap decision(const char* action, const QString& label, const QString& message = QString())
{
    QVariantMap result;
    result[QString::fromLatin1(WorkoutStartPolicy::actionKey)] = QString::fromLatin1(action);
    result[QString::fromLatin1(WorkoutStartPolicy::labelKey)] = label;
    result[QString::fromLatin1(WorkoutStartPolicy::messageKey)] = message;
    return result;
}

QString startLabel() { return QStringLiteral("Start workout"); }

QString continueLabel() { return QStringLiteral("Continue workout"); }

bool hasMoment(const QDateTime& moment)
{
    return moment.isValid() && moment.toMSecsSinceEpoch() > 0;
}

bool isSameWorkout(const WorkoutModel& selected, const WorkoutModel& active)
{
    if (&selected == &active)
        return true;
    return selected.id() > 0 && selected.id() == active.id();
}

bool isFinished(const WorkoutModel& workout)
{
    return workout.status() == WorkoutStatus::Ended || hasMoment(workout.endedTime());
}

QString finishedMessage(const WorkoutModel& workout)
{
    const QString name = workout.name().trimmed();
    const QString subject
        = name.isEmpty() ? QStringLiteral("That workout") : QStringLiteral("\"%1\"").arg(name);
    const QString day
        = QLocale::c().toString(workout.endedTime().date(), QStringLiteral("d MMM yyyy"));
    const QString when
        = hasMoment(workout.endedTime()) ? QStringLiteral(" on %1").arg(day) : QString();

    return QStringLiteral("%1 was finished%2, so it cannot be started again.\n\n"
                          "Open the workouts tab, expand it under History and tap the repeat "
                          "button — that plans a fresh copy you can start.")
        .arg(subject, when);
}

}  // namespace

WorkoutStartPolicy::WorkoutStartPolicy(QObject* parent)
    : QObject(parent)
{
}

QVariantMap WorkoutStartPolicy::decide(WorkoutModel* selected, WorkoutModel* active) const
{
    if (!selected)
        return decision(missingAction, startLabel());

    if (active && isSameWorkout(*selected, *active))
        return decision(resumeAction, continueLabel());

    if (isFinished(*selected))
        return decision(blockedAction, startLabel(), finishedMessage(*selected));

    if (active)
        return decision(replaceAction, startLabel());

    return decision(startAction, startLabel());
}
