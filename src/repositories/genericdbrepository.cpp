#include "genericdbrepository.h"
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include "../utils/serializationutils.h"

GenericDbRepository::GenericDbRepository(const QString &tableName, const QStringList &keys, QSqlDatabase &database, QObject *parent)
    : QObject(parent), m_tableName(tableName), m_keys(keys), m_database(database) {
}

bool GenericDbRepository::createTable(const QString &queryStr)
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    return query.exec(queryStr);
}

void GenericDbRepository::initialize(const QString &jsonFile)
{
    if (loadAll().isEmpty()) {
        QVector<QVariantMap> initialWorkouts = loadFromFile(jsonFile, m_keys);
        saveAll(initialWorkouts);
    }
}

QVariantMap GenericDbRepository::loadModel() {
    return loadAll("LIMIT 1").first();
}

int GenericDbRepository::add(const QVariantMap &item)
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    QStringList commonKeys;
    for (const QString &key : m_keys) {
        if (item.contains(key)) {
            commonKeys.append(key);
        }
    }

    if (commonKeys.isEmpty()) {
        qWarning() << "No common keys between table columns and item data";
        return -1;
    }

    QString insertQuery = QString("INSERT INTO %1 (%2) VALUES (%3)")
                              .arg(m_tableName)
                              .arg(commonKeys.join(", "))
                              .arg(QString("?,").repeated(commonKeys.size()).chopped(1));

    query.prepare(insertQuery);
    for (const QString &key : commonKeys) {
        query.addBindValue(item.value(key));
    }

    if (!query.exec()) {
        qWarning() << "Error inserting into table" << m_tableName << ":" << query.lastError();
        qWarning() << "Query:" << query.lastQuery();
        qWarning() << "Bound values:" << query.boundValues();
        return -1;
    }

    return query.lastInsertId().toInt();
}

int GenericDbRepository::save(const QVariantMap &item)
{
    if (!item.contains("id") || (item.contains("id") && item.value("id").toInt() == -1)) {
        return add(item);
    }
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);

    QStringList keysWithoutId = m_keys;
    keysWithoutId.removeAll("id");

    QString updateQuery = QString("INSERT OR REPLACE INTO %1 (%2) VALUES (%3)")
                          .arg(m_tableName)
                          .arg(m_keys.join(", "))
                          .arg(QString("?,").repeated(m_keys.size()).chopped(1));
    query.prepare(updateQuery);
    for (const QString &key : m_keys) {
        query.addBindValue(item.value(key));
    }
    if (!query.exec()) {
        qWarning() << "Error saving into table" << m_tableName << ":" << query.lastError();
        qWarning() << "Prepared query:" << query.lastQuery();
        qWarning() << "Number of bound values:" << query.boundValues().size();
        qWarning() << "Expected placeholders:" << m_keys.size();
        qWarning() << "Item data:" << item;
        return -1;
    }

    return item["id"].toInt();
}

bool GenericDbRepository::remove(const QString &condition)
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);

    QString deleteQuery = QString("DELETE FROM %1 %2").arg(m_tableName).arg(condition);

    if (!query.exec(deleteQuery)) {
        qWarning() << "Error deleting from table" << m_tableName << ":" << query.lastError();
        qWarning() << "Query:" << deleteQuery;
        return false;
    }

    qDebug() << "Deleted" << query.numRowsAffected() << "rows from table" << m_tableName;
    return true;
}

void GenericDbRepository::saveAll(const QVector<QVariantMap> &items) {
    for (const QVariantMap &item : items) {
        save(item);
    }
}

QVector<QVariantMap> GenericDbRepository::loadAll(const QString &condition)
{
    QVector<QVariantMap> items;
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    QString selectQuery = QString("SELECT %1 FROM %2")
                          .arg(m_keys.join(", ")).arg(m_tableName);

    if (!condition.isEmpty()) {
        selectQuery += " " + condition;
    }

    if (query.exec(selectQuery)) {
        while (query.next()) {
            QVariantMap item;
            for (int i = 0; i < m_keys.size(); ++i) {
                item[m_keys[i]] = query.value(i);
            }
            items.append(item);
        }
    } else {
        qWarning() << "Error loading all items from table" << m_tableName << ":" << query.lastError();
        qWarning() << "Query:" << selectQuery;
    }
    return items;
}

QVector<QVariantMap> GenericDbRepository::loadFromFile(const QString &path, const QStringList &keys)
{
    QVector<QVariantMap> items;
    QFile loadFile(path);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file for reading:" << path;
        return items;
    }

    QByteArray fileData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(fileData));
    auto jsonArray = loadDoc.array();

    for (const auto &item : jsonArray) {
        if (item.isObject()) {
            QVariantMap fullMap = Serialization::fromJson(item.toObject());
            QVariantMap filteredMap;

            for (const QString &key : keys) {
                if (fullMap.contains(key))
                    filteredMap[key] = fullMap[key];
            }

            items.append(filteredMap);
        }
    }

    return items;
}

void GenericDbRepository::saveToFile(const QString &path,
                                     const QVector<QVariantMap> &items,
                                     const QStringList &keys)
{
    QFile saveFile(path);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file for writing:" << path;
        return;
    }

    QJsonArray jsonArray;
    for (const auto &item : items) {
        QVariantMap filteredMap;
        for (const QString &key : keys) {
            if (item.contains(key))
                filteredMap[key] = item[key];
        }
        jsonArray.append(Serialization::toJson(filteredMap));
    }

    QJsonDocument saveDoc(jsonArray);
    saveFile.write(saveDoc.toJson());
}
