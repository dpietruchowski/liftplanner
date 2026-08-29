#include "workouttext.h"
#include <QRegularExpression>
#include <QStringList>

namespace WorkoutText
{

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

QString formatDistance(double meters)
{
    if (meters < 1000.0)
        return QString("%1 m").arg(qRound(meters));

    const double kilometers = meters / 1000.0;
    return QString("%1 km").arg(
        QString::number(kilometers, 'f', 1).remove(QRegularExpression("\\.0$")));
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
        header.append(QString("%1 - %2").arg(started.toString("HH:mm"),
                                             ended.isValid() ? ended.toString("HH:mm") : QString()));
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
