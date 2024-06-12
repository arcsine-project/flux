#pragma once
#if !__has_builtin(__builtin_bit_cast) && __cpp_lib_bit_cast >= 201806L
#    include <bit>
#endif

namespace flux::fou {

// clang-format off
template <typename R, typename... Args>
struct [[nodiscard]] make_noexcept final {};

template <typename R, typename... Args>
struct [[nodiscard]] make_noexcept<R(Args...)> final {
    using type = R(Args...) noexcept;
};

template <typename R, typename... Args>
struct [[nodiscard]] make_noexcept<R(Args...) noexcept> final {
    using type = R(Args...) noexcept;
};

template <typename R, typename... Args>
using make_noexcept_t = typename make_noexcept<R, Args...>::type;

template <meta::function F>
FLUX_ALWAYS_INLINE
inline constexpr auto noexcept_cast(F* f) noexcept {
#if __has_builtin(__builtin_bit_cast)
    return __builtin_bit_cast(make_noexcept_t<F>*, f);
#elif __cpp_lib_bit_cast >= 201806L
    return ::std::bit_cast<make_noexcept_t<F>*>(f);
#else
    return reinterpret_cast<make_noexcept_t<F>*>(f);
#endif
}

template <meta::function F, typename... Args>
FLUX_ALWAYS_INLINE
inline constexpr decltype(auto) noexcept_call(F* f, Args&&... args) noexcept {
    if consteval {
        return f(::std::forward<Args>(args)...); // EH unwinding does not matter here
    } else {
        return noexcept_cast(f)(::std::forward<Args>(args)...);
    }
}
// clang-format on

} // namespace flux::fou