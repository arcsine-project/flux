#pragma once

namespace flux::fou {

namespace detail {

// clang-format off
struct [[nodiscard]] distance_fn final {
    template <typename I, meta::sentinel_for<I> S>
        requires(not meta::sized_sentinel_for<S, I>)
    static constexpr meta::iter_diff_t<I> operator()(I first, S last) noexcept {
        meta::iter_diff_t<I> n = 0;
        while (first != last) {
            ++first;
            ++n;
        }
        return n;
    }

    template <typename I, meta::sized_sentinel_for<meta::decay_t<I>> S>
    static constexpr auto operator()(I&& first, S const last) noexcept
            -> meta::iter_diff_t<meta::decay_t<I>> {
        return last - static_cast<meta::decay_t<I> const&>(first);
    }

    template <meta::range Range>
    static constexpr meta::range_diff_t<Range> operator()(Range&& range) noexcept {
        if constexpr (meta::sized_range<Range>) {
            return static_cast<meta::range_diff_t<Range>>(::std::ranges::size(range));
        } else {
            return operator()(::std::ranges::begin(range), ::std::ranges::end(range));
        }
    }
};
// clang-format on

} // namespace detail

namespace ranges {
inline constexpr auto distance = detail::distance_fn{};
} // namespace ranges

namespace detail {

struct [[nodiscard]] sized_distance_fn final {
    template <typename I, meta::sentinel_for<I> S>
        requires(not meta::sized_sentinel_for<S, I>)
    FLUX_ALWAYS_INLINE static constexpr auto operator()(I first, S last) noexcept {
        return static_cast<::std::size_t>(ranges::distance(::std::move(first), ::std::move(last)));
    }

    template <typename I, meta::sized_sentinel_for<meta::decay_t<I>> S>
    FLUX_ALWAYS_INLINE static constexpr auto operator()(I&& first, S const last) noexcept {
        return static_cast<::std::size_t>(ranges::distance(::std::move(first), ::std::move(last)));
    }

    template <meta::range Range>
    FLUX_ALWAYS_INLINE static constexpr auto operator()(Range&& range) noexcept {
        return static_cast<::std::size_t>(ranges::distance(::std::move(range)));
    }
};

} // namespace detail

namespace ranges {
inline constexpr auto sized_distance = detail::sized_distance_fn{};
} // namespace ranges

} // namespace flux::fou