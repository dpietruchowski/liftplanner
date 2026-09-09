#pragma once

#include <QString>

enum class Equipment
{
    Barbell,
    Dumbbell,
    Kettlebell,
    Machine,
    Cable,
    SmithMachine,
    TrapBar,
    Bodyweight,
    Band,
    Sled,
    Treadmill,
    Bike,
    Rower,
    Other
};

inline QString equipmentToString(Equipment equipment)
{
    switch (equipment)
    {
        case Equipment::Barbell:
            return QStringLiteral("barbell");
        case Equipment::Dumbbell:
            return QStringLiteral("dumbbell");
        case Equipment::Kettlebell:
            return QStringLiteral("kettlebell");
        case Equipment::Machine:
            return QStringLiteral("machine");
        case Equipment::Cable:
            return QStringLiteral("cable");
        case Equipment::SmithMachine:
            return QStringLiteral("smith_machine");
        case Equipment::TrapBar:
            return QStringLiteral("trap_bar");
        case Equipment::Bodyweight:
            return QStringLiteral("bodyweight");
        case Equipment::Band:
            return QStringLiteral("band");
        case Equipment::Sled:
            return QStringLiteral("sled");
        case Equipment::Treadmill:
            return QStringLiteral("treadmill");
        case Equipment::Bike:
            return QStringLiteral("bike");
        case Equipment::Rower:
            return QStringLiteral("rower");
        case Equipment::Other:
            return QStringLiteral("other");
    }
    return QStringLiteral("other");
}

inline Equipment equipmentFromString(const QString& str)
{
    if (str == QStringLiteral("barbell"))
        return Equipment::Barbell;
    if (str == QStringLiteral("dumbbell"))
        return Equipment::Dumbbell;
    if (str == QStringLiteral("kettlebell"))
        return Equipment::Kettlebell;
    if (str == QStringLiteral("machine"))
        return Equipment::Machine;
    if (str == QStringLiteral("cable"))
        return Equipment::Cable;
    if (str == QStringLiteral("smith_machine"))
        return Equipment::SmithMachine;
    if (str == QStringLiteral("trap_bar"))
        return Equipment::TrapBar;
    if (str == QStringLiteral("bodyweight"))
        return Equipment::Bodyweight;
    if (str == QStringLiteral("band"))
        return Equipment::Band;
    if (str == QStringLiteral("sled"))
        return Equipment::Sled;
    if (str == QStringLiteral("treadmill"))
        return Equipment::Treadmill;
    if (str == QStringLiteral("bike"))
        return Equipment::Bike;
    if (str == QStringLiteral("rower"))
        return Equipment::Rower;
    return Equipment::Other;
}
