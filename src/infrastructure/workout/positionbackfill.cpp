#include "positionbackfill.h"

#include <dbtoolkit/query/order.h>
#include <dbtoolkit/query/select.h>
#include <dbtoolkit/query/update.h>
#include <dbtoolkit/query/where.h>

Order positionOrder(const QString& parentKey, const QString& positionKey)
{
    return Order(parentKey).asc().then(positionKey).asc();
}

bool backfillPositions(QSqlDatabase& database, const QString& table, const QString& parentKey,
                       const QString& idKey, const QString& positionKey)
{
    const QVector<QVariantMap> rows = Select(QStringList { idKey, parentKey })
                                          .from(table)
                                          .orderBy(Order(parentKey).asc().then(idKey).asc())
                                          .execute(database);

    int currentParent = -1;
    int position = 0;

    for (const QVariantMap& row : rows)
    {
        const int parent = row.value(parentKey).toInt();
        if (parent != currentParent)
        {
            currentParent = parent;
            position = 0;
        }

        const int affected = Update(table)
                                 .set(positionKey, position)
                                 .where(Where(idKey).equals(row.value(idKey).toInt()))
                                 .execute(database)
                                 .toInt();
        if (affected < 0)
            return false;

        ++position;
    }

    return true;
}
