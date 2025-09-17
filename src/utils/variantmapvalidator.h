#pragma once

#include <QString>
#include <QUrl>
#include <QVariantMap>

class VariantMapValidator
{
public:
    VariantMapValidator(const QVariantMap &variantMap, QString &errorString);

    bool validateString(const QString &key, bool required = false);
    bool validateInt(const QString &key, bool required = false, int minValue = 0);
    bool validateDouble(const QString &key, bool required = false, double minValue = 0.0);
    bool validateUrl(const QString &key, bool required = false);

    bool hasError() const { return !m_errorString.isEmpty(); }
    QString errorString() const { return m_errorString; }

private:
    const QVariantMap &m_variantMap;
    QString &m_errorString;
};
