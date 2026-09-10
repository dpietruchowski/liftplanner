#include "blanksession.h"
#include <QLocale>

namespace
{

constexpr auto blank_session_prefix = "Freestyle";

}

QString blankSessionName(const QDate& day)
{
    const QString prefix = QString::fromLatin1(blank_session_prefix);
    if (!day.isValid())
        return prefix;

    return QStringLiteral("%1 · %2").arg(prefix,
                                         QLocale::c().toString(day, QStringLiteral("d MMM")));
}

Workout blankSession(const QDateTime& moment)
{
    Workout session(blankSessionName(moment.date()), moment);
    session.setPlannedTime(moment);
    return session;
}
