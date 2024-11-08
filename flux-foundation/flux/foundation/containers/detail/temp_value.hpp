#pragma once

namespace flux::fou::detail {

// clang-format off
template <typename T, typename Allocator>
struct temp_value final {
    using allocator_traits = allocator_traits<Allocator>;

    union {
        T value;
    };
    Allocator& allocator;

    template <typename... Args>
    FLUX_NO_CFI constexpr explicit temp_value(Allocator& alloc, Args&&... args) noexcept
            : allocator(alloc) {
        construct_at(addressof(value), ::std::forward<Args>(args)...);
    }

    temp_value(temp_value const&)            = delete;
    temp_value& operator=(temp_value const&) = delete;

    constexpr ~temp_value() requires meta::trivially_destructible<T> = default;
    constexpr ~temp_value() { destroy_at(addressof(value)); }

    constexpr T& get() noexcept {
        return value;
    }

    constexpr T const& get() const noexcept {
        return value;
    }
};
// clang-format on

} // namespace flux::fou::detail