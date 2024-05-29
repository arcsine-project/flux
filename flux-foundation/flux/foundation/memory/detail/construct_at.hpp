#pragma once

// This is a workaround for writing your own constexpr `construct_at` if you don't want to
// pull everything found in the <memory> header. Currently, this hack only works with Clang.
// But that's okay, since I'm not going to use the others.
namespace std::detail {

// clang-format off
template <typename T, typename... Args>
    requires requires(void* ptr, Args&&... args) { ::new (ptr) T(static_cast<Args&&>(args)...); }
    // ^^^ allows copy elision since C++17 mandates it
constexpr T* construct_at(T* const location, Args&&... args) noexcept {
    FLUX_ASSERT(location != nullptr, "null pointer given to construct_at");
    // @see: https://cplusplus.github.io/LWG/issue3870
    return ::new (static_cast<void*>(location)) T(::std::forward<Args>(args)...);
}
// clang-format on

} // namespace std::detail

namespace flux::fou {

template <meta::forward_iterator Iterator>
constexpr Iterator destroy_range(Iterator first, Iterator last) noexcept;

namespace detail {

using ::std::detail::construct_at;

// clang-format off
template <typename T>
#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#endif
constexpr void* voidify(T& from) noexcept {
    // Cast away cv-qualifiers to allow modifying elements of a range through const iterators.
    return const_cast<void*>(static_cast<void const volatile*>(addressof(from)));
}

template <typename T>
constexpr void destroy_at(T* const location) noexcept {
    FLUX_ASSERT(location != nullptr, "null pointer given to destroy_at");
    if constexpr (meta::is_array_v<T>) {
        destroy_range(::std::begin(*location), ::std::end(*location));
    } else if constexpr(not meta::trivially_destructible<T>) {
        location->~T();
    }
}
// clang-format on

} // namespace detail

// clang-format off
template <typename T>
constexpr T* default_construct_at(T* const p) noexcept {
    if consteval {
        return detail::construct_at(p);
    }
    return ::new (static_cast<void*>(p)) T;
}

template <typename T>
constexpr T* value_construct_at(T* const p) noexcept {
    return detail::construct_at(p);
}
// clang-format on

} // namespace flux::fou