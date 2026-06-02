#include "workouttext.h"
#include <QStringList>

namespace WorkoutText
{

QString workoutToText(const Workout& workout)
{
    QDateTime date = workout.startedTime();
    if (!date.isValid())
        date = workout.endedTime();
    if (!date.isValid())
        date = workout.createdTime();

    QStringList lines;
    lines.append("---");
    lines.append(QString("%1,%2").arg(workout.name(), date.toString("yyyy-MM-dd")));
    lines.append(QString());

    for (const auto& exercise : workout.exercises())
        lines.append(QString("%1,%2").arg(exercise.name(), exercise.setsToString()));

    return lines.join("\n");
}

}  // namespace WorkoutText
