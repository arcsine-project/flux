#pragma once

namespace flux::fou {

struct nontrivial_type {
    // This default constructor is user-provided to avoid
    // zero-initialization when objects are value-initialized.
    constexpr nontrivial_type() noexcept {}
};

// clang-format off
template <typename T>
union [[nodiscard, clang::trivial_abi]] uninitialized_storage final {
    using value_type = T;

    value_type      value;
    nontrivial_type _;

    constexpr uninitialized_storage() : _{} {}

    constexpr uninitialized_storage(uninitialized_storage const& other) noexcept
        requires meta::trivially_copy_constructible<T>
    = default;
    constexpr uninitialized_storage(uninitialized_storage const& other) noexcept
            : value{other.value} {}

    constexpr uninitialized_storage(uninitialized_storage&& other) noexcept
        requires meta::trivially_move_constructible<T>
    = default;
    constexpr uninitialized_storage(uninitialized_storage&& other) noexcept
            : value{std::move(other.value)} {}

    constexpr uninitialized_storage& operator=(uninitialized_storage const&) noexcept
        requires meta::trivially_copy_assignable<T>
    = default;
    constexpr uninitialized_storage& operator=(uninitialized_storage const&) noexcept = delete;

    constexpr uninitialized_storage& operator=(uninitialized_storage&&) noexcept
        requires meta::trivially_move_assignable<T>
    = default;
    constexpr uninitialized_storage& operator=(uninitialized_storage&&) noexcept = delete;

    constexpr ~uninitialized_storage() requires meta::trivially_destructible<T> = default;
    constexpr ~uninitialized_storage() {}

    [[nodiscard]] constexpr value_type* data() noexcept {
        return addressof(value);
    }
    [[nodiscard]] constexpr value_type const* data() const noexcept {
        return addressof(value);
    }
};
// clang-format on

template <typename T>
constexpr bool operator==(uninitialized_storage<T> const& lhs,
                          uninitialized_storage<T> const& rhs) noexcept {
    return lhs.value == rhs.value;
}

} // namespace flux::fou

namespace flux {

// clang-format off
template <meta::reference Reference, typename T>
constexpr Reference get(fou::uninitialized_storage<T>& storage) noexcept {
    return *storage.data();
}
template <meta::reference Reference, typename T>
constexpr Reference get(fou::uninitialized_storage<T> const& storage) noexcept {
    return *storage.data();
}
template <meta::reference Reference, typename T>
constexpr T&& get(T&& value) noexcept {
    return value;
}

template <meta::pointer Pointer, typename T>
constexpr Pointer get(fou::uninitialized_storage<T>& storage) noexcept {
    return storage.data();
}
template <meta::pointer Pointer, typename T>
constexpr Pointer get(fou::uninitialized_storage<T> const& storage) noexcept {
    return storage.data();
}
template <meta::pointer Pointer, typename T>
constexpr T* get(T& value) noexcept {
    return fou::addressof(value);
}
// clang-format on

// Optional, since types with trivial lifetimes will not be wrapped by an uninitialized storage.
template <typename T>
using optional_uninitialized_storage =
        meta::condition<meta::has_trivial_lifetime<T>, T, fou::uninitialized_storage<T>>;

} // namespace flux