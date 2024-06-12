#pragma once
#if defined(__cpp_lib_to_address) && __cpp_lib_to_address >= 201711L
#    include <memory>
#endif

namespace flux::fou::detail {

// clang-format off
template <typename T>
#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#endif
constexpr T* to_address(T* const pointer) noexcept {
    static_assert(not meta::function<T>, "T is a function type.");
    return pointer;
}

template <typename T>
#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#endif
constexpr auto to_address(T const& pointer) noexcept {
#if defined(__cpp_lib_to_address) && __cpp_lib_to_address >= 201711L
    return ::std::to_address(pointer);
#else
    return addressof(*pointer);
#endif
}
// clang-format on

} // namespace flux::fou::detail