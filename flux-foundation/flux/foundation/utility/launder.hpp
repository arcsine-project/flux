#pragma once

namespace flux::fou {

// clang-format off
template <typename T>
#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#endif
[[nodiscard]] inline constexpr T* launder(T* p) noexcept {
    return __builtin_launder(p);
}
// clang-format on

} // namespace flux::fou