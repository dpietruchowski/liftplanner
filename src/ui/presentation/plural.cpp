#include "ui/presentation/plural.h"

namespace Plural
{

QString form(int count, const QString& singular, const QString& plural)
{
    return count == 1 ? singular : plural;
}

QString counted(int count, const QString& singular, const QString& plural)
{
    return QStringLiteral("%1 %2").arg(count).arg(form(count, singular, plural));
}

}

PluralText::PluralText(QObject* parent)
    : QObject(parent)
{
}

QString PluralText::form(int count, const QString& singular, const QString& plural) const
{
    return Plural::form(count, singular, plural);
}

QString PluralText::counted(int count, const QString& singular, const QString& plural) const
{
    return Plural::counted(count, singular, plural);
}
