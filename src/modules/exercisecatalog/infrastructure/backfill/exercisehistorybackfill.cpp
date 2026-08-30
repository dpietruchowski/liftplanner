#include "exercisehistorybackfill.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionquery.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionrepository.h"
#include "modules/exercisecatalog/domain/services/exercisematcher.h"
#include "modules/workout/infrastructure/serializers/exerciseserializer.h"

#include <dbtoolkit/query/select.h>
#include <dbtoolkit/query/update.h>
#include <dbtoolkit/query/where.h>

#include <QHash>

HistoryBackfillResult ExerciseHistoryBackfill::apply(QSqlDatabase& database,
                                                     const ExerciseDefinitionRepository& catalog)
{
    HistoryBackfillResult result;

    const QVector<QVariantMap> rows
        = Select(QStringList { ExerciseSerializer::id_key, ExerciseSerializer::name_key })
              .from(ExerciseSerializer::table)
              .where(Where(ExerciseSerializer::definition_id_key).isNull())
              .execute(database);

    result.examined = rows.size();
    if (rows.isEmpty())
        return result;

    const auto candidates = ExerciseMatcher::toCandidates(
        catalog.findAll(ExerciseDefinitionQuery().whereArchived(false)));

    QHash<QString, std::optional<int>> resolved;

    for (const QVariantMap& row : rows)
    {
        const QString name = row.value(ExerciseSerializer::name_key).toString();

        auto it = resolved.find(name);
        if (it == resolved.end())
            it = resolved.insert(name, ExerciseMatcher::match(name, candidates));

        if (!it.value().has_value())
        {
            ++result.unmatched;
            continue;
        }

        const int affected = Update(ExerciseSerializer::table)
                                 .set(ExerciseSerializer::definition_id_key, it.value().value())
                                 .where(Where(ExerciseSerializer::id_key)
                                            .equals(row.value(ExerciseSerializer::id_key).toInt()))
                                 .execute(database)
                                 .toInt();

        if (affected > 0)
            ++result.linked;
        else
            ++result.unmatched;
    }

    return result;
}
