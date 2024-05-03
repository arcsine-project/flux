#pragma once

namespace flux::fou {

// clang-format off
template <typename T>
#if __has_cpp_attribute(clang::always_inline)
[[clang::always_inline]]
#endif
[[nodiscard]] inline constexpr T* launder(T* p) noexcept {
    return __builtin_launder(p);
}
// clang-format on

} // namespace flux::fou