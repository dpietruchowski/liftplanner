#pragma once

#include "domain/workout/loadtype.h"
#include "domain/workout/setmetric.h"

namespace StrengthMath
{

bool isWeighted(SetMetric metric, LoadType loadType);
double volume(int repetitions, double weight);
double oneRepMax(int repetitions, double weight);

}
