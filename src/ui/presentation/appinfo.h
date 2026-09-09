#pragma once
#include <QObject>
#include <QString>

class AppInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
public:
    explicit AppInfo(QObject* parent = nullptr);

    QString name() const;
    QString version() const;
    QString qtVersion() const;

    Q_INVOKABLE QString licenseText(const QString& license) const;
};
