#include "generatednamebackfill.h"

#include "domain/workout/generatedworkoutname.h"
#include "infrastructure/workout/workoutserializer.h"

#include <dbtoolkit/query/select.h>
#include <dbtoolkit/query/update.h>
#include <dbtoolkit/query/where.h>

#include <QDateTime>
#include <QRegularExpression>

namespace
{

bool looksLikeADayStampedName(const QString& name)
{
    static const QRegularExpression pattern(
        QStringLiteral("^%1 · [0-9]{1,2} [A-Z][a-z]{2}$").arg(generatedPlanName()));

    return pattern.match(name).hasMatch();
}

QString repairedName(const QDateTime& startedTime)
{
    return startedTime.isValid() ? generatedSessionName(startedTime) : generatedPlanName();
}

}

bool backfillGeneratedNames(QSqlDatabase& database)
{
    const QVector<QVariantMap> rows
        = Select(QStringList { WorkoutSerializer::id_key, WorkoutSerializer::name_key,
                               WorkoutSerializer::started_time_key })
              .from(WorkoutSerializer::table)
              .execute(database);

    for (const QVariantMap& row : rows)
    {
        if (!looksLikeADayStampedName(row.value(WorkoutSerializer::name_key).toString()))
            continue;

        const QDateTime startedTime = QDateTime::fromString(
            row.value(WorkoutSerializer::started_time_key).toString(), Qt::ISODate);

        const int affected
            = Update(WorkoutSerializer::table)
                  .set(WorkoutSerializer::name_key, repairedName(startedTime))
                  .set(WorkoutSerializer::generated_name_key, 1)
                  .where(Where(WorkoutSerializer::id_key)
                             .equals(row.value(WorkoutSerializer::id_key).toInt()))
                  .execute(database)
                  .toInt();

        if (affected < 0)
            return false;
    }

    return true;
}
