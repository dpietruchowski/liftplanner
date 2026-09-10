#pragma once

#include <QChar>
#include <QString>

namespace SetNotation
{

inline constexpr char16_t completed_marker = u'!';

inline QString marked(const QString& token, bool completed)
{
    return completed ? token + QChar(completed_marker) : token;
}

inline bool takeCompletedMarker(QString& token)
{
    if (!token.endsWith(QChar(completed_marker)))
        return false;

    token.chop(1);
    return true;
}

}
