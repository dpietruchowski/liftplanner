#pragma once

#include <QJsonObject>
#include <QMetaEnum>
#include <QString>
#include <QVariant>
#include <string>
#include <tuple>

#define DEFINE_KEY(field_name) static constexpr const char *field_name##_key = #field_name;
#define DEFINE_TABLE_NAME(table_name) static constexpr const char *table_name##_table = #table_name;

#define DECLARE_MODEL_PROPERTY(type, name, setterName, keyName) \
private: \
    type m_##name{}; \
\
public: \
    Q_PROPERTY(type name READ name WRITE setterName NOTIFY name##Changed) \
    type name() const \
    { \
        return m_##name; \
    } \
    void setterName(type value) \
    { \
        if (m_##name != value) { \
            m_##name = value; \
            emit name##Changed(); \
        } \
    } \
    DEFINE_KEY(keyName) \
    Q_SIGNAL void name##Changed();

namespace Serialization {
QJsonObject toJson(const QVariantMap &variantMap);
QVariantMap fromJson(const QJsonObject &jsonObj);

template<typename EnumType>
QString convertEnumToString(int enumValue)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<EnumType>();
    return metaEnum.valueToKey(enumValue);
    }
    template <typename EnumType>
    EnumType convertStringToEnum(const QString &enumName)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<EnumType>();
        return static_cast<EnumType>(metaEnum.keyToValue(enumName.toUtf8().constData()));
    }
    } // namespace Serialization
