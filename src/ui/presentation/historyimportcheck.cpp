#include "historyimportcheck.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

namespace HistoryImportCheck
{

QString clipboardProblem(const QString& clipboardText)
{
    if (!clipboardText.trimmed().isEmpty())
        return QString();

    return QStringLiteral("Import failed: the clipboard is empty. Copy the workout history JSON "
                          "first, then tap import again.");
}

QString payloadProblem(const QString& jsonData)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
        return QStringLiteral("Import failed: the clipboard does not hold valid JSON (%1). Copy "
                              "the whole JSON block from your AI chat.")
            .arg(parseError.errorString());

    if (!doc.isArray())
        return QStringLiteral("Import failed: workout history has to be a JSON array of workouts.");

    const QJsonArray workouts = doc.array();
    if (workouts.isEmpty())
        return QStringLiteral("Import failed: that JSON array holds no workouts.");

    for (int index = 0; index < workouts.size(); ++index)
    {
        const QJsonValue entry = workouts.at(index);
        if (!entry.isObject() || !entry.toObject().value(QStringLiteral("name")).isString())
            return QStringLiteral("Import failed: entry %1 of that array is not a workout - it "
                                  "has no \"name\".")
                .arg(index + 1);
    }

    return QString();
}

}
