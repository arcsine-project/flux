#pragma once
#if defined(_MSC_VER) && !defined(FLUX_CLANG)
#    include <cstdlib>
#endif

namespace flux::fou {

[[noreturn]] inline void unreachable() noexcept {
#if __has_builtin(__builtin_unreachable)
    __builtin_unreachable();
#else
    __assume(false);
#endif
}

} // namespace flux::fou
