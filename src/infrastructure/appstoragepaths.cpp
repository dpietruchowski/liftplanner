#include "appstoragepaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace AppStoragePaths
{

QString directory()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    return path;
}

QString file(const QString& name) { return directory() + QLatin1Char('/') + name; }

QString database(const QString& name)
{
    const QString path = file(name);
    if (QFile::exists(path) || !QFile::exists(name))
        return path;

    if (QFile::copy(name, path))
        qInfo().noquote() << QStringLiteral(
                                 "Adopted the database next to the application: %1 -> %2")
                                 .arg(QFileInfo(name).absoluteFilePath(), path);
    else
        qWarning().noquote()
            << QStringLiteral("Could not adopt the database next to the application: %1").arg(name);

    return path;
}

}
