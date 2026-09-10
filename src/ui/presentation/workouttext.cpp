#include "workouttext.h"
#include "ui/presentation/plural.h"
#include <QLocale>
#include <QRegularExpression>
#include <QStringList>

namespace WorkoutText
{

QString finishPrompt(int completedSets, int plannedSets)
{
    const QString sets = Plural::form(plannedSets, QStringLiteral("set"), QStringLiteral("sets"));

    if (completedSets >= plannedSets)
    {
        return QStringLiteral("All %1 %2 ticked off. End this workout?")
            .arg(Plural::counted(plannedSets, QStringLiteral("set"), QStringLiteral("sets")),
                 Plural::form(plannedSets, QStringLiteral("is"), QStringLiteral("are")));
    }

    return QStringLiteral("%1 of %2 %3 %4 ticked off. The rest stay marked as not done.")
        .arg(completedSets)
        .arg(plannedSets)
        .arg(sets, Plural::form(completedSets, QStringLiteral("is"), QStringLiteral("are")));
}

QString amnestyLossWarning(const QString& workoutName, int plannedSets)
{
    const QString name = workoutName.trimmed().isEmpty()
        ? QStringLiteral("This session")
        : QStringLiteral("\"%1\"").arg(workoutName.trimmed());

    const int rest = plannedSets > 1 ? plannedSets - 1 : 0;

    return QStringLiteral("%1 has no ticked sets, so the whole session counts as done. "
                          "Ticking this one marks the other %2 as skipped.")
        .arg(name, Plural::counted(rest, QStringLiteral("set"), QStringLiteral("sets")));
}

QString plannedDayLabel(const QDate& day, const QDate& today)
{
    if (!day.isValid())
        return QStringLiteral("No day set");

    const QString date = QLocale::c().toString(day, QStringLiteral("ddd, d MMM yyyy"));
    const qint64 distance = today.isValid() ? today.daysTo(day) : 1;

    if (distance == 0)
        return QStringLiteral("Today · %1").arg(date);
    if (distance == 1)
        return QStringLiteral("Tomorrow · %1").arg(date);

    return date;
}

QString formatDuration(qint64 seconds)
{
    qint64 hours = seconds / 3600;
    qint64 minutes = (seconds % 3600) / 60;
    if (hours > 0)
        return QString("%1h %2m").arg(hours).arg(minutes, 2, 10, QChar('0'));
    if (minutes > 0)
        return QString("%1m").arg(minutes);
    return QString("%1s").arg(seconds);
}

QString formatRest(int seconds)
{
    const int safe = seconds > 0 ? seconds : 0;
    return QStringLiteral("%1:%2").arg(safe / 60).arg(safe % 60, 2, 10, QChar('0'));
}

QString formatDistance(double meters)
{
    if (meters < 1000.0)
        return QString("%1 m").arg(qRound(meters));

    const double kilometers = meters / 1000.0;
    return QString("%1 km").arg(
        QString::number(kilometers, 'f', 1).remove(QRegularExpression("\\.0$")));
}

QString formatVolume(double kilograms)
{
    const qint64 rounded = kilograms > 0.0 ? qRound64(kilograms) : 0;

    QString digits = QString::number(rounded);
    for (int at = digits.size() - 3; at > 0; at -= 3)
        digits.insert(at, QChar(' '));

    return digits + QStringLiteral(" kg");
}

QString workoutToText(const Workout& workout)
{
    const QDateTime& started = workout.startedTime();
    const QDateTime& ended = workout.endedTime();

    QDateTime date = started.isValid() ? started : ended;
    if (!date.isValid())
        date = workout.createdTime();

    QStringList header;
    header.append(workout.name());
    header.append(date.toString("yyyy-MM-dd"));
    if (started.isValid())
        header.append(QString("%1 - %2").arg(
            started.toString("HH:mm"), ended.isValid() ? ended.toString("HH:mm") : QString()));
    if (started.isValid() && ended.isValid())
        header.append(formatDuration(started.secsTo(ended)));

    QStringList lines;
    lines.append("---");
    lines.append(header.join(", "));
    lines.append(QString());

    for (const auto& exercise : workout.exercises())
        lines.append(QString("%1, %2").arg(exercise.name(), exercise.setsToString()));

    return lines.join("\n");
}

}  // namespace WorkoutText
