#pragma once

#include <QString>

class Order;
class QSqlDatabase;

Order positionOrder(const QString& parentKey, const QString& positionKey);

bool backfillPositions(QSqlDatabase& database, const QString& table, const QString& parentKey,
                       const QString& idKey, const QString& positionKey);
