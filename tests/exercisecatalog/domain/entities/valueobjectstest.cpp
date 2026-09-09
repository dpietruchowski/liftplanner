#include "domain/exercisecatalog/catalogorigin.h"
#include "domain/exercisecatalog/equipment.h"
#include "domain/exercisecatalog/laterality.h"
#include "domain/exercisecatalog/mechanics.h"
#include "domain/exercisecatalog/muscle.h"
#include "domain/exercisecatalog/muscleinvolvement.h"
#include "domain/exercisecatalog/musclerole.h"
#include <QSet>
#include <gtest/gtest.h>
#include <vector>

namespace
{

std::vector<Muscle> allMuscles()
{
    return { Muscle::Chest,      Muscle::Lats,       Muscle::UpperBack, Muscle::LowerBack,
             Muscle::Traps,      Muscle::FrontDelts, Muscle::SideDelts, Muscle::RearDelts,
             Muscle::Biceps,     Muscle::Triceps,    Muscle::Forearms,  Muscle::Quads,
             Muscle::Hamstrings, Muscle::Glutes,     Muscle::Adductors, Muscle::Abductors,
             Muscle::Calves,     Muscle::HipFlexors, Muscle::Abs,       Muscle::Obliques,
             Muscle::Neck };
}

std::vector<Equipment> allEquipment()
{
    return { Equipment::Barbell, Equipment::Dumbbell,   Equipment::Kettlebell,
             Equipment::Machine, Equipment::Cable,      Equipment::SmithMachine,
             Equipment::TrapBar, Equipment::Bodyweight, Equipment::Band,
             Equipment::Sled,    Equipment::Treadmill,  Equipment::Bike,
             Equipment::Rower,   Equipment::Other };
}

std::vector<BodyRegion> allRegions()
{
    return { BodyRegion::Chest, BodyRegion::Back, BodyRegion::Shoulders, BodyRegion::Arms,
             BodyRegion::Legs,  BodyRegion::Core, BodyRegion::Neck };
}

}

class MuscleTest : public ::testing::Test
{
};

TEST_F(MuscleTest, EveryMuscleRoundTripsThroughItsKey)
{
    for (Muscle muscle : allMuscles())
        EXPECT_EQ(muscleFromString(muscleToString(muscle)), muscle);
}

TEST_F(MuscleTest, TableCoversEveryMuscleExactlyOnce)
{
    EXPECT_EQ(muscleEntries.size(), allMuscles().size());

    QSet<QString> keys;
    for (const auto& entry : muscleEntries)
        keys.insert(QString::fromLatin1(entry.key));

    EXPECT_EQ(keys.size(), static_cast<int>(muscleEntries.size()));
}

TEST_F(MuscleTest, UnknownKeyFallsBackToChest)
{
    EXPECT_EQ(muscleFromString("not_a_muscle"), Muscle::Chest);
    EXPECT_EQ(muscleFromString(""), Muscle::Chest);
}

TEST_F(MuscleTest, RegionIsDerivedNotStored)
{
    EXPECT_EQ(regionOf(Muscle::Chest), BodyRegion::Chest);
    EXPECT_EQ(regionOf(Muscle::Lats), BodyRegion::Back);
    EXPECT_EQ(regionOf(Muscle::LowerBack), BodyRegion::Back);
    EXPECT_EQ(regionOf(Muscle::RearDelts), BodyRegion::Shoulders);
    EXPECT_EQ(regionOf(Muscle::Triceps), BodyRegion::Arms);
    EXPECT_EQ(regionOf(Muscle::Hamstrings), BodyRegion::Legs);
    EXPECT_EQ(regionOf(Muscle::HipFlexors), BodyRegion::Legs);
    EXPECT_EQ(regionOf(Muscle::Obliques), BodyRegion::Core);
    EXPECT_EQ(regionOf(Muscle::Neck), BodyRegion::Neck);
}

TEST_F(MuscleTest, EveryRegionIsReachableFromSomeMuscle)
{
    QSet<int> reached;
    for (Muscle muscle : allMuscles())
        reached.insert(static_cast<int>(regionOf(muscle)));

    EXPECT_EQ(reached.size(), static_cast<int>(allRegions().size()));
}

TEST_F(MuscleTest, EveryRegionRoundTripsThroughItsKey)
{
    for (BodyRegion region : allRegions())
        EXPECT_EQ(bodyRegionFromString(bodyRegionToString(region)), region);
}

class MuscleRoleTest : public ::testing::Test
{
};

TEST_F(MuscleRoleTest, RoundTripsAndDefaultsToPrimary)
{
    EXPECT_EQ(muscleRoleFromString(muscleRoleToString(MuscleRole::Primary)), MuscleRole::Primary);
    EXPECT_EQ(muscleRoleFromString(muscleRoleToString(MuscleRole::Secondary)),
              MuscleRole::Secondary);
    EXPECT_EQ(muscleRoleFromString(muscleRoleToString(MuscleRole::Stabilizer)),
              MuscleRole::Stabilizer);
    EXPECT_EQ(muscleRoleFromString("nonsense"), MuscleRole::Primary);
}

class MuscleInvolvementTest : public ::testing::Test
{
};

TEST_F(MuscleInvolvementTest, CarriesMuscleAndRoleTogether)
{
    MuscleInvolvement involvement { Muscle::Quads, MuscleRole::Primary };

    EXPECT_EQ(involvement.muscle, Muscle::Quads);
    EXPECT_EQ(involvement.role, MuscleRole::Primary);
}

TEST_F(MuscleInvolvementTest, EqualityComparesBothFields)
{
    MuscleInvolvement a { Muscle::Quads, MuscleRole::Primary };
    MuscleInvolvement b { Muscle::Quads, MuscleRole::Primary };
    MuscleInvolvement differentRole { Muscle::Quads, MuscleRole::Secondary };
    MuscleInvolvement differentMuscle { Muscle::Glutes, MuscleRole::Primary };

    EXPECT_EQ(a, b);
    EXPECT_NE(a, differentRole);
    EXPECT_NE(a, differentMuscle);
}

class EquipmentTest : public ::testing::Test
{
};

TEST_F(EquipmentTest, EveryValueRoundTripsThroughItsKey)
{
    for (Equipment equipment : allEquipment())
        EXPECT_EQ(equipmentFromString(equipmentToString(equipment)), equipment);
}

TEST_F(EquipmentTest, UnknownKeyFallsBackToOther)
{
    EXPECT_EQ(equipmentFromString("nautilus"), Equipment::Other);
}

TEST_F(EquipmentTest, KeysAreDistinct)
{
    QSet<QString> keys;
    for (Equipment equipment : allEquipment())
        keys.insert(equipmentToString(equipment));

    EXPECT_EQ(keys.size(), static_cast<int>(allEquipment().size()));
}

class MechanicsAndLateralityTest : public ::testing::Test
{
};

TEST_F(MechanicsAndLateralityTest, RoundTripWithCompoundAndBilateralAsDefaults)
{
    EXPECT_EQ(mechanicsFromString(mechanicsToString(Mechanics::Compound)), Mechanics::Compound);
    EXPECT_EQ(mechanicsFromString(mechanicsToString(Mechanics::Isolation)), Mechanics::Isolation);
    EXPECT_EQ(mechanicsFromString("nonsense"), Mechanics::Compound);

    EXPECT_EQ(lateralityFromString(lateralityToString(Laterality::Bilateral)),
              Laterality::Bilateral);
    EXPECT_EQ(lateralityFromString(lateralityToString(Laterality::Unilateral)),
              Laterality::Unilateral);
    EXPECT_EQ(lateralityFromString("nonsense"), Laterality::Bilateral);
}

class CatalogOriginTest : public ::testing::Test
{
};

TEST_F(CatalogOriginTest, DistinguishesMachineCreatedFromUserCreated)
{
    EXPECT_EQ(catalogOriginFromString(catalogOriginToString(CatalogOrigin::BuiltIn)),
              CatalogOrigin::BuiltIn);
    EXPECT_EQ(catalogOriginFromString(catalogOriginToString(CatalogOrigin::Custom)),
              CatalogOrigin::Custom);
    EXPECT_EQ(catalogOriginFromString(catalogOriginToString(CatalogOrigin::Imported)),
              CatalogOrigin::Imported);

    EXPECT_NE(catalogOriginToString(CatalogOrigin::Custom),
              catalogOriginToString(CatalogOrigin::Imported));
}

TEST_F(CatalogOriginTest, UnknownKeyFallsBackToBuiltIn)
{
    EXPECT_EQ(catalogOriginFromString("seeded"), CatalogOrigin::BuiltIn);
}
