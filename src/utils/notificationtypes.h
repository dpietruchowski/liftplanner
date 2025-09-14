#pragma once
#include <QObject>

namespace Notification
{
    Q_NAMESPACE

    enum class Type
    {
        Error,
        Info,
        Warning,
        Success
    };
    Q_ENUM_NS(Type)

    enum class Button { None = 0x0, Ok = 0x1, Cancel = 0x2 };
    Q_ENUM_NS(Button)
    Q_DECLARE_FLAGS(Buttons, Button)
    Q_FLAG_NS(Buttons)

    } // namespace Notification
    Q_DECLARE_OPERATORS_FOR_FLAGS(Notification::Buttons)
