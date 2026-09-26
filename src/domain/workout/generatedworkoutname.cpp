#include "generatedworkoutname.h"
#include <QLocale>

namespace
{

constexpr auto generated_name_stem = "Freestyle";

}

QString generatedPlanName() { return QString::fromLatin1(generated_name_stem); }

QString generatedSessionName(const QDateTime& startedAt)
{
    if (!startedAt.isValid())
        return generatedPlanName();

    return QStringLiteral("%1 · %2").arg(
        generatedPlanName(), QLocale::c().toString(startedAt.time(), QStringLiteral("HH:mm")));
}
