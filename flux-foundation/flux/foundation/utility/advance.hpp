#pragma once

namespace flux::fou {

namespace detail {

struct [[nodiscard]] advance_fn final {
    // Preconditions: If `I` does not model `bidirectional_iterator`, `n` is not negative.
    template <meta::input_or_output_iterator I>
    static constexpr void operator()(I& i, meta::iter_diff_t<I> n) noexcept {
        // If `I` models `random_access_iterator`, equivalent to `i += n`.
        if constexpr (meta::random_access_iterator<I>) {
            i += n;
        } else if constexpr (meta::bidirectional_iterator<I>) {
            // Otherwise, if `n` is non-negative, increments `i` by `n`.
            advance_forward(i, n);
            // Otherwise, decrements `i` by `-n`.
            advance_backward(i, n);
        } else {
            FLUX_ASSERT(n >= 0, "negative advance of non-bidirectional iterator");

            // Otherwise, if `n` is non-negative, increments `i` by `n`.
            advance_forward(i, n);
        }
    }

    // Preconditions: Either `assignable_from<I&, S> || sized_sentinel_for<S, I>` is modeled, or [i,
    // bound) denotes a range.
    template <meta::input_or_output_iterator I, meta::sentinel_for<I> S>
    static constexpr void operator()(I& i, S bound) noexcept {
        // If `I` and `S` model `assignable_from<I&, S>`, equivalent to `i = std::move(bound)`.
        if constexpr (meta::assignable_from<I&, S>) {
            i = static_cast<S&&>(bound);
        } else if constexpr (meta::sized_sentinel_for<S, I>) {
            // Otherwise, if `S` and `I` model `sized_sentinel_for<S, I>`, equivalent to
            // `ranges::advance(i, bound - i)`.
            operator()(i, bound - i);
        } else {
            // Otherwise, while `bool(i != bound)` is true, increments `i`.
            while (i != bound) {
                ++i;
            }
        }
    }

    // Preconditions:
    //   * If `n > 0`, [i, bound) denotes a range.
    //   * If `n == 0`, [i, bound) or [bound, i) denotes a range.
    //   * If `n < 0`, [bound, i) denotes a range, `I` models `bidirectional_iterator`, and `I` and
    //   `S` model `same_as<I, S>`.
    // Returns: `n - M`, where `M` is the difference between the ending and starting position.
    template <meta::input_or_output_iterator I, meta::sentinel_for<I> S>
    static constexpr auto operator()(I& i, meta::iter_diff_t<I> n, S bound) noexcept {
        // If `S` and `I` model `sized_sentinel_for<S, I>`:
        if constexpr (meta::sized_sentinel_for<S, I>) {
            // If |n| >= |bound - i|, equivalent to `ranges::advance(i, bound)`.
            const meta::iter_diff_t<I> delta = bound - i;
            if ((n < 0 && n <= delta) || (n > 0 && n >= delta)) {
                operator()(i, bound);
                return n - delta;
            }

            // Otherwise, equivalent to `ranges::advance(i, n)`.
            operator()(i, n);
            return meta::iter_diff_t<I>(0);
        } else {
            // Otherwise, while `bool(i != bound_sentinel)` is true, decrements `i` but at most `-n`
            // times.
            if constexpr (meta::bidirectional_iterator<I> && meta::same_as<I, S>) {
                for (; n < 0 && i != bound; ++n) {
                    --i;
                }
            } else {
                FLUX_ASSERT(n >= 0, "negative advance of non-bidirectional iterator");
            }

            // Otherwise, if `n` is non-negative, while `bool(i != bound)` is true, increments `i`
            // but at most `n` times.
            for (; n > 0 && i != bound; --n) {
                ++i;
            }
            return n;
        }
    }

private:
    // clang-format off
    template <typename I>
#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#endif
    static constexpr void advance_forward(I& i, meta::iter_diff_t<I> n) noexcept {
        while (n > 0) {
            --n;
            ++i;
        }
    }

    template <typename I>
#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#endif
    static constexpr void advance_backward(I& i, meta::iter_diff_t<I> n) noexcept {
        while (n < 0) {
            ++n;
            --i;
        }
    }
    // clang-format on
};

} // namespace detail

namespace ranges {
inline constexpr auto advance = detail::advance_fn{};
} // namespace ranges

} // namespace flux::fou