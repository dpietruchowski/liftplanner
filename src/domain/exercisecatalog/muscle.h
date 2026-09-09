#pragma once

#include <QString>
#include <array>

enum class Muscle
{
    Chest,
    Lats,
    UpperBack,
    LowerBack,
    Traps,
    FrontDelts,
    SideDelts,
    RearDelts,
    Biceps,
    Triceps,
    Forearms,
    Quads,
    Hamstrings,
    Glutes,
    Adductors,
    Abductors,
    Calves,
    HipFlexors,
    Abs,
    Obliques,
    Neck
};

enum class BodyRegion
{
    Chest,
    Back,
    Shoulders,
    Arms,
    Legs,
    Core,
    Neck
};

struct MuscleEntry
{
    Muscle muscle;
    const char* key;
    BodyRegion region;
};

inline constexpr std::array<MuscleEntry, 21> muscleEntries { {
    { Muscle::Chest, "chest", BodyRegion::Chest },
    { Muscle::Lats, "lats", BodyRegion::Back },
    { Muscle::UpperBack, "upper_back", BodyRegion::Back },
    { Muscle::LowerBack, "lower_back", BodyRegion::Back },
    { Muscle::Traps, "traps", BodyRegion::Back },
    { Muscle::FrontDelts, "front_delts", BodyRegion::Shoulders },
    { Muscle::SideDelts, "side_delts", BodyRegion::Shoulders },
    { Muscle::RearDelts, "rear_delts", BodyRegion::Shoulders },
    { Muscle::Biceps, "biceps", BodyRegion::Arms },
    { Muscle::Triceps, "triceps", BodyRegion::Arms },
    { Muscle::Forearms, "forearms", BodyRegion::Arms },
    { Muscle::Quads, "quads", BodyRegion::Legs },
    { Muscle::Hamstrings, "hamstrings", BodyRegion::Legs },
    { Muscle::Glutes, "glutes", BodyRegion::Legs },
    { Muscle::Adductors, "adductors", BodyRegion::Legs },
    { Muscle::Abductors, "abductors", BodyRegion::Legs },
    { Muscle::Calves, "calves", BodyRegion::Legs },
    { Muscle::HipFlexors, "hip_flexors", BodyRegion::Legs },
    { Muscle::Abs, "abs", BodyRegion::Core },
    { Muscle::Obliques, "obliques", BodyRegion::Core },
    { Muscle::Neck, "neck", BodyRegion::Neck },
} };

inline QString muscleToString(Muscle muscle)
{
    for (const auto& entry : muscleEntries)
    {
        if (entry.muscle == muscle)
            return QString::fromLatin1(entry.key);
    }
    return QStringLiteral("chest");
}

inline Muscle muscleFromString(const QString& str)
{
    for (const auto& entry : muscleEntries)
    {
        if (str == QLatin1String(entry.key))
            return entry.muscle;
    }
    return Muscle::Chest;
}

inline BodyRegion regionOf(Muscle muscle)
{
    for (const auto& entry : muscleEntries)
    {
        if (entry.muscle == muscle)
            return entry.region;
    }
    return BodyRegion::Core;
}

inline QString bodyRegionToString(BodyRegion region)
{
    switch (region)
    {
        case BodyRegion::Chest:
            return QStringLiteral("chest");
        case BodyRegion::Back:
            return QStringLiteral("back");
        case BodyRegion::Shoulders:
            return QStringLiteral("shoulders");
        case BodyRegion::Arms:
            return QStringLiteral("arms");
        case BodyRegion::Legs:
            return QStringLiteral("legs");
        case BodyRegion::Core:
            return QStringLiteral("core");
        case BodyRegion::Neck:
            return QStringLiteral("neck");
    }
    return QStringLiteral("core");
}

inline BodyRegion bodyRegionFromString(const QString& str)
{
    if (str == QStringLiteral("chest"))
        return BodyRegion::Chest;
    if (str == QStringLiteral("back"))
        return BodyRegion::Back;
    if (str == QStringLiteral("shoulders"))
        return BodyRegion::Shoulders;
    if (str == QStringLiteral("arms"))
        return BodyRegion::Arms;
    if (str == QStringLiteral("legs"))
        return BodyRegion::Legs;
    if (str == QStringLiteral("neck"))
        return BodyRegion::Neck;
    return BodyRegion::Core;
}
