#pragma once

#include <QObject>
#include <QString>

#define DECLARE_PROPERTY(type, name, setterName)                          \
private:                                                                  \
    type m_##name{};                                                      \
                                                                          \
public:                                                                   \
    Q_PROPERTY(type name READ name WRITE setterName NOTIFY name##Changed) \
    type name() const                                                     \
    {                                                                     \
        return m_##name;                                                  \
    }                                                                     \
    void setterName(type value)                                           \
    {                                                                     \
        if (m_##name != value)                                            \
        {                                                                 \
            m_##name = value;                                             \
            emit name##Changed();                                         \
        }                                                                 \
    }                                                                     \
    Q_SIGNAL void name##Changed();
