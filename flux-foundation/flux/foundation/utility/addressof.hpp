#pragma once

namespace flux::fou {

// clang-format off
template <typename T>
#if __has_cpp_attribute(clang::always_inline)
[[clang::always_inline]]
#endif
[[nodiscard]] inline constexpr T* addressof(T& arg) noexcept {
    return __builtin_addressof(arg);
}

template <typename T>
T const* addressof(T const&&) = delete;
// clang-format on

} // namespace flux::fou