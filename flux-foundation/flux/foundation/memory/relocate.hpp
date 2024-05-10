#pragma once
#include <flux/foundation/memory/detail/constexpr_memcpy.hpp>

namespace flux::fou {

// clang-format off
template <typename T>
struct [[nodiscard]] destroy_guard final {
    T* value;
    constexpr explicit destroy_guard(T* const other) noexcept : value{other} {}
    constexpr ~destroy_guard() noexcept requires meta::trivially_destructible<T> = default;
    constexpr ~destroy_guard() noexcept { detail::destroy_at(value); }
};

template <typename T, typename U>
constexpr U* relocate_at(T* const src, U* const dest) noexcept {
    if constexpr (meta::same_trivially_relocatable<T, U>) {
        if consteval {
            destroy_guard<T> guard{src};
            return detail::construct_at(dest, ::std::move(*src));
        } else {
            detail::constexpr_memmove(dest, src);
            return launder(dest); // required?
        }
    } else {
        destroy_guard<T> guard{src};
        return detail::construct_at(dest, ::std::move(*src));
    }
}
// clang-format on

} // namespace flux::fou