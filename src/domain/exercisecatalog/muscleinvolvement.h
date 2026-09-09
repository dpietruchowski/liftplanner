#pragma once

#include "muscle.h"
#include "musclerole.h"

struct MuscleInvolvement
{
    Muscle muscle { Muscle::Chest };
    MuscleRole role { MuscleRole::Primary };
};

inline bool operator==(const MuscleInvolvement& lhs, const MuscleInvolvement& rhs)
{
    return lhs.muscle == rhs.muscle && lhs.role == rhs.role;
}

inline bool operator!=(const MuscleInvolvement& lhs, const MuscleInvolvement& rhs)
{
    return !(lhs == rhs);
}
