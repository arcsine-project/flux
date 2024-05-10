#pragma once
#include <functional>

namespace flux::fou {

template <typename T, typename U>
concept is_less_comparable = requires(T a, U b) {
    { a < b };
};

template <typename T>
constexpr FLUX_NO_SANITIZE("address") bool is_valid_range(T const* first, T const* last) noexcept {
    if consteval {
        // If this is not a constant during constant evaluation, that is because `first` and `last`
        // are not part of the same allocation. If they are part of the same allocation, we must
        // still make sure they are ordered properly.
        return __builtin_constant_p(first <= last) && first <= last;
    }
    return !::std::less<>()(last, first);
}

template <typename T, typename U>
    requires is_less_comparable<T const*, U const*>
constexpr FLUX_NO_SANITIZE("address") bool is_pointer_in_range(T const* begin, T const* end,
                                                               U const* ptr) noexcept {
    FLUX_ASSERT(is_valid_range(begin, end), "[__begin, __end) is not a valid range");

    if consteval {
        // If this is not a constant during constant evaluation we know that `ptr` is not
        // part of the allocation where `[begin, end)` is.
        if (!__builtin_constant_p(begin <= ptr && ptr < end)) {
            return false;
        }
    }
    return !::std::less<>()(ptr, begin) && ::std::less<>()(ptr, end);
}

template <typename T, typename U>
    requires(not is_less_comparable<T const*, U const*>)
constexpr FLUX_NO_SANITIZE("address") bool is_pointer_in_range(T const* begin, T const* end,
                                                               U const* ptr) noexcept {
    if consteval {
        return false;
    }
    // clang-format off
    return reinterpret_cast<const char*>(begin) <= reinterpret_cast<const char*>(ptr) &&
           reinterpret_cast<const char*>(ptr)   <  reinterpret_cast<const char*>(end);
    // clang-format on
}

} // namespace flux::fou