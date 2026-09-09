#include "strengthmath.h"

namespace StrengthMath
{

bool isWeighted(SetMetric metric, LoadType loadType)
{
    return metric == SetMetric::Reps && loadType == LoadType::External;
}

double volume(int repetitions, double weight) { return repetitions * weight; }

double oneRepMax(int repetitions, double weight)
{
    constexpr int brzycki_limit = 10;
    constexpr double brzycki_numerator = 36.0;
    constexpr double brzycki_offset = 37.0;
    constexpr double epley_divisor = 30.0;

    if (repetitions <= 0)
        return 0.0;
    if (repetitions == 1)
        return weight;
    if (repetitions <= brzycki_limit)
        return weight * brzycki_numerator / (brzycki_offset - repetitions);
    return weight * (1.0 + repetitions / epley_divisor);
}

}
