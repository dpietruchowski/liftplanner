#include "appinfo.h"

#include <QCoreApplication>
#include <QFile>
#include <QStringList>

namespace
{
QString reflowParagraphs(const QString& text)
{
    QStringList paragraphs;
    for (const QString& paragraph : text.split(QStringLiteral("\n\n")))
    {
        QStringList lines;
        for (const QString& line : paragraph.split(QLatin1Char('\n')))
        {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty())
                lines.append(trimmed);
        }
        if (!lines.isEmpty())
            paragraphs.append(lines.join(QLatin1Char(' ')));
    }
    return paragraphs.join(QStringLiteral("\n\n"));
}
}

AppInfo::AppInfo(QObject* parent)
    : QObject(parent)
{
}

QString AppInfo::name() const { return QCoreApplication::applicationName(); }

QString AppInfo::version() const { return QCoreApplication::applicationVersion(); }

QString AppInfo::qtVersion() const { return QString::fromLatin1(qVersion()); }

QString AppInfo::licenseText(const QString& license) const
{
    QFile file(QStringLiteral(":/LiftPlanner/data/%1.txt").arg(license));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return reflowParagraphs(QString::fromUtf8(file.readAll()));
}
