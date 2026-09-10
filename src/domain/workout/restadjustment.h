#pragma once

#include <algorithm>
#include <cstdlib>

namespace RestAdjustment
{
inline constexpr int stepSeconds = 15;
inline constexpr int minimumSeconds = 15;
inline constexpr int maximumSeconds = 600;

inline int clamped(int seconds) { return std::clamp(seconds, minimumSeconds, maximumSeconds); }

inline int nextUp(int seconds) { return clamped((seconds / stepSeconds + 1) * stepSeconds); }

inline int nextDown(int seconds)
{
    return clamped(((seconds + stepSeconds - 1) / stepSeconds - 1) * stepSeconds);
}

inline int adjusted(int seconds, int steps)
{
    int result = clamped(seconds);
    for (int i = 0; i < std::abs(steps); ++i)
        result = steps > 0 ? nextUp(result) : nextDown(result);

    return result;
}
}
