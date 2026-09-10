#pragma once

#include <QObject>
#include <QString>

namespace Plural
{

QString form(int count, const QString& singular, const QString& plural);
QString counted(int count, const QString& singular, const QString& plural);

}

class PluralText : public QObject
{
    Q_OBJECT

public:
    explicit PluralText(QObject* parent = nullptr);

    Q_INVOKABLE QString form(int count, const QString& singular, const QString& plural) const;
    Q_INVOKABLE QString counted(int count, const QString& singular, const QString& plural) const;
};
