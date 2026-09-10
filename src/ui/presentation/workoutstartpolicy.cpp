#include "ui/presentation/workoutstartpolicy.h"

#include "domain/workout/sessionsummary.h"
#include "ui/models/workoutmodel.h"
#include <QDateTime>
#include <QLocale>

namespace
{

QVariantMap decision(const char* action, const QString& label, const QString& message = QString(),
                     const QString& confirmation = QString())
{
    QVariantMap result;
    result[QString::fromLatin1(WorkoutStartPolicy::actionKey)] = QString::fromLatin1(action);
    result[QString::fromLatin1(WorkoutStartPolicy::labelKey)] = label;
    result[QString::fromLatin1(WorkoutStartPolicy::messageKey)] = message;
    result[QString::fromLatin1(WorkoutStartPolicy::confirmationKey)] = confirmation;
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

QString quotedName(const WorkoutModel& workout, const QString& fallback)
{
    const QString name = workout.name().trimmed();
    return name.isEmpty() ? fallback : QStringLiteral("\"%1\"").arg(name);
}

QString replaceConfirmation(const WorkoutModel& selected, const WorkoutModel& active)
{
    const QString running = quotedName(active, QStringLiteral("The running workout"));
    const QString next = quotedName(selected, QStringLiteral("the selected workout"));
    const Workout entity = active.toEntity();

    if (!completionFlagsAreMeaningful(entity))
    {
        return QStringLiteral("%1 is still running, but no set is ticked off yet.\n\n"
                              "Starting %2 now puts it back on your planned list, "
                              "so nothing is lost.")
            .arg(running, next);
    }

    const SessionSummary summary = summarizeSession(entity);
    const QString progress
        = QStringLiteral("%1 of %2 %3")
              .arg(summary.completedSets)
              .arg(summary.plannedSets)
              .arg(summary.plannedSets == 1 ? QStringLiteral("set") : QStringLiteral("sets"));
    const QString kept
        = summary.completedSets == 1 ? QStringLiteral("that set") : QStringLiteral("those sets");

    return QStringLiteral("%1 is still running with %2 ticked off.\n\n"
                          "Starting %3 now ends it and files it in your history with %4.")
        .arg(running, progress, next, kept);
}

QString finishedMessage(const WorkoutModel& workout)
{
    const QString subject = quotedName(workout, QStringLiteral("That workout"));
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
        return decision(replaceAction, startLabel(), QString(),
                        replaceConfirmation(*selected, *active));

    return decision(startAction, startLabel());
}
