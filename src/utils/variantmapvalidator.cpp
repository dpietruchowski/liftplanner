#include "variantmapvalidator.h"

VariantMapValidator::VariantMapValidator(const QVariantMap &variantMap, QString &errorString)
    : m_variantMap(variantMap)
    , m_errorString(errorString)
{}

bool VariantMapValidator::validateString(const QString &key, bool required)
{
    if (!m_variantMap.contains(key)) {
        if (required) {
            m_errorString = QString("Missing '%1' field").arg(key);
            return false;
        }
        return true;
    }

    if (!m_variantMap.value(key).canConvert<QString>()) {
        m_errorString = QString("Invalid '%1' field").arg(key);
        return false;
    }

    if (required) {
        QString value = m_variantMap.value(key).toString().trimmed();
        if (value.isEmpty()) {
            m_errorString = QString("'%1' cannot be empty").arg(key);
            return false;
        }
    }

    return true;
}

bool VariantMapValidator::validateInt(const QString &key, bool required, int minValue)
{
    if (!m_variantMap.contains(key)) {
        if (required) {
            m_errorString = QString("Missing '%1' field").arg(key);
            return false;
        }
        return true;
    }

    if (!m_variantMap.value(key).canConvert<int>()) {
        m_errorString = QString("Invalid '%1' field").arg(key);
        return false;
    }

    int value = m_variantMap.value(key).toInt();
    if (value < minValue) {
        m_errorString = QString("'%1' must be >= %2").arg(key).arg(minValue);
        return false;
    }

    return true;
}

bool VariantMapValidator::validateDouble(const QString &key, bool required, double minValue)
{
    if (!m_variantMap.contains(key)) {
        if (required) {
            m_errorString = QString("Missing '%1' field").arg(key);
            return false;
        }
        return true;
    }

    if (!m_variantMap.value(key).canConvert<double>()) {
        m_errorString = QString("Invalid '%1' field").arg(key);
        return false;
    }

    double value = m_variantMap.value(key).toDouble();
    if (value < minValue) {
        m_errorString = QString("'%1' must be >= %2").arg(key).arg(minValue);
        return false;
    }

    return true;
}

bool VariantMapValidator::validateUrl(const QString &key, bool required)
{
    if (!m_variantMap.contains(key)) {
        if (required) {
            m_errorString = QString("Missing '%1' field").arg(key);
            return false;
        }
        return true;
    }

    if (!m_variantMap.value(key).canConvert<QString>()) {
        m_errorString = QString("Invalid '%1' field").arg(key);
        return false;
    }

    QString urlValue = m_variantMap.value(key).toString().trimmed();
    if (required && urlValue.isEmpty()) {
        m_errorString = QString("'%1' cannot be empty").arg(key);
        return false;
    }

    if (!urlValue.isEmpty()) {
        QUrl url(urlValue);
        if (!url.isValid() || url.scheme().isEmpty()) {
            m_errorString = QString("'%1' must be a valid URL").arg(key);
            return false;
        }
    }

    return true;
}
