#pragma once
#include <QObject>
#include <QVariantMap>
#include <QVector>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

class GenericDbRepository : public QObject {
    Q_OBJECT

public:
    explicit GenericDbRepository(const QString &tableName, const QStringList &keys, QSqlDatabase &database, QObject *parent = nullptr);

    bool createTable(const QString &queryStr);
    void initialize(const QString &jsonFile);

    QVariantMap loadModel();
    int add(const QVariantMap &item);
    int save(const QVariantMap &item);
    bool remove(const QString &condition);
    void saveAll(const QVector<QVariantMap> &items);
    QVector<QVariantMap> loadAll(const QString &condition = QString());

    QVector<QVariantMap> loadFromFile(const QString &path, const QStringList &keys);
    void saveToFile(const QString &path, const QVector<QVariantMap> &items, const QStringList &keys);

protected:
    QString m_tableName;
    QStringList m_keys;
    QSqlDatabase &m_database;
    QMutex m_mutex;
};
