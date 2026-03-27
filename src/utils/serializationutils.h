#pragma once

#include <QObject>
#include <QQmlListProperty>
#include <QString>

#define DEFINE_KEY(field_name) static constexpr const char *field_name##_key = #field_name;

#define DECLARE_MODEL_PROPERTY(type, name, setterName, keyName)           \
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
    DEFINE_KEY(keyName)                                                   \
    Q_SIGNAL void name##Changed();

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

#define DECLARE_MODEL_LIST_PROPERTY(classType, type, name, setterName, keyName)      \
private:                                                                             \
    QList<type *> m_##name;                                                          \
                                                                                     \
public:                                                                              \
    Q_PROPERTY(QQmlListProperty<type> name READ name##Property NOTIFY name##Changed) \
                                                                                     \
    QQmlListProperty<type> name##Property()                                          \
    {                                                                                \
        return QQmlListProperty<type>(this, this,                                    \
                                      &classType::append##name,                      \
                                      &classType::count##name,                       \
                                      &classType::at##name,                          \
                                      &classType::clear##name);                      \
    }                                                                                \
                                                                                     \
    QList<type *> name() const                                                       \
    {                                                                                \
        return m_##name;                                                             \
    }                                                                                \
                                                                                     \
    void setterName(const QList<type *> &list)                                       \
    {                                                                                \
        if (m_##name != list)                                                        \
        {                                                                            \
            qDeleteAll(m_##name);                                                    \
            m_##name = list;                                                         \
            for (auto *obj : m_##name)                                               \
            {                                                                        \
                if (!obj->parent())                                                  \
                    obj->setParent(this);                                            \
            }                                                                        \
            emit name##Changed();                                                    \
        }                                                                            \
    }                                                                                \
                                                                                     \
    DEFINE_KEY(keyName)                                                              \
    Q_SIGNAL void name##Changed();                                                   \
                                                                                     \
private:                                                                             \
    static void append##name(QQmlListProperty<type> *prop, type *obj)                \
    {                                                                                \
        classType *w = static_cast<classType *>(prop->object);                       \
        if (w && obj)                                                                \
        {                                                                            \
            obj->setParent(w);                                                       \
            w->m_##name.append(obj);                                                 \
            emit w->name##Changed();                                                 \
        }                                                                            \
    }                                                                                \
    static qsizetype count##name(QQmlListProperty<type> *prop)                       \
    {                                                                                \
        return static_cast<classType *>(prop->object)->m_##name.count();             \
    }                                                                                \
    static type *at##name(QQmlListProperty<type> *prop, qsizetype index)             \
    {                                                                                \
        return static_cast<classType *>(prop->object)->m_##name.at(index);           \
    }                                                                                \
    static void clear##name(QQmlListProperty<type> *prop)                            \
    {                                                                                \
        classType *w = static_cast<classType *>(prop->object);                       \
        if (w)                                                                       \
        {                                                                            \
            qDeleteAll(w->m_##name);                                                 \
            w->m_##name.clear();                                                     \
            emit w->name##Changed();                                                 \
        }                                                                            \
    }
