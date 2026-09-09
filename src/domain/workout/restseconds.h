#pragma once

namespace RestSeconds
{

inline constexpr int inherited = -1;

inline constexpr bool isInherited(int override) { return override < 0; }

inline constexpr int effective(int override, int fallback)
{
    return isInherited(override) ? fallback : override;
}

}
