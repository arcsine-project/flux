#pragma once

namespace flux::fou {

namespace detail {

struct [[nodiscard]] next_fn final {
    // clang-format off
    template <meta::input_or_output_iterator I>
    static constexpr I operator()(I i) noexcept {
        ++i;
        return i;
    }
    // clang-format on

    template <meta::input_or_output_iterator I>
    static constexpr I operator()(I i, meta::iter_diff_t<I> n) noexcept {
        ranges::advance(i, n);
        return i;
    }

    template <meta::input_or_output_iterator I, meta::sentinel_for<I> S>
    static constexpr I operator()(I i, S bound) noexcept {
        ranges::advance(i, bound);
        return i;
    }

    template <meta::input_or_output_iterator I, meta::sentinel_for<I> S>
    static constexpr I operator()(I i, meta::iter_diff_t<I> n, S bound) noexcept {
        ranges::advance(i, n, bound);
        return i;
    }
};

} // namespace detail

namespace ranges {
inline constexpr auto next = detail::next_fn{};
} // namespace ranges

} // namespace flux::fou