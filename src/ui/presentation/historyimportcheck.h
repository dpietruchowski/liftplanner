#pragma once

#include <QString>

namespace HistoryImportCheck
{

QString clipboardProblem(const QString& clipboardText);
QString payloadProblem(const QString& jsonData);

}
