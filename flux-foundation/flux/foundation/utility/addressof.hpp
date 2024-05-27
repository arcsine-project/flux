#pragma once

namespace flux::fou {

// clang-format off
template <typename T>
#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#endif
[[nodiscard]] inline constexpr T* addressof(T& arg) noexcept {
    return __builtin_addressof(arg);
}

template <typename T>
T const* addressof(T const&&) = delete;
// clang-format on

} // namespace flux::fou