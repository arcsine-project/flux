#pragma once
#include <flux/meta.hpp>

#include <flux/foundation/memory/detail/constexpr_memcpy.hpp>
#include <flux/foundation/utility/distance.hpp>

namespace flux::fou {

namespace ranges {
using ::std::ranges::equal_to;
} // namespace ranges

namespace detail {

using ::std::identity;

template <typename T>
concept is_identity__ = meta::same_as<T, identity>;

template <typename T>
concept is_equal_to__ = meta::same_as<T, ranges::equal_to>;

template <typename T, typename U, typename Pred, typename Proj1, typename Proj2>
concept use_memcpm__ = is_equal_to__<Pred> and is_identity__<Proj1> and is_identity__<Proj2> and
                       meta::not_volatile<T> and meta::not_volatile<U> and
                       meta::is_trivially_equality_comparable_v<T, U>;

// clang-format off
struct [[nodiscard]] equal_fn final {
    template <meta::input_iterator Iter1, meta::sentinel_for<Iter1> Sent1,
              meta::input_iterator Iter2, meta::sentinel_for<Iter2> Sent2,
              typename Pred  = ranges::equal_to,
              typename Proj1 = identity, typename Proj2 = identity>
        requires meta::indirectly_comparable<Iter1, Iter2, Pred, Proj1, Proj2>
    static constexpr bool operator()(Iter1 first1, Sent1 last1, Iter2 first2, Sent2 last2,
                                     Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) noexcept {
        if constexpr (meta::sized_sentinel_for<Sent1, Iter1> and
                      meta::sized_sentinel_for<Sent2, Iter2>) {
            auto d1 = ranges::distance(first1, last1);
            auto d2 = ranges::distance(first2, last2);
            if (d1 != d2) return false;

            if constexpr (use_memcpm__<Iter1, Iter2, Pred, Proj1, Proj2>) {
                return constexpr_memcmp_equal(first1, first2, static_cast<::std::size_t>(d1));
            } else {
                for (; first1 != last1; ++first1, (void)++first2) {
                    if (!(bool)::std::invoke(pred,
                                             ::std::invoke(proj1, *first1),
                                             ::std::invoke(proj2, *first2)))
                        return false;
                }
                return true;
            }
        } else {
            for (; first1 != last1; ++first1, (void)++first2) {
                if (!(bool)::std::invoke(pred,
                                         ::std::invoke(proj1, *first1),
                                         ::std::invoke(proj2, *first2)))
                    return false;
            }
            return first1 == last1 && first2 == last2;
        }
    }

    template <meta::input_range Range1, meta::input_range Range2,
              typename Pred  = ranges::equal_to,
              typename Proj1 = identity, typename Proj2 = identity>
        requires meta::indirectly_comparable<meta::iterator_t<Range1>, meta::iterator_t<Range2>,
                                             Pred, Proj1, Proj2>
    FLUX_ALWAYS_INLINE
    static constexpr bool operator()(Range1&& r1, Range2&& r2, Pred pred = {},
                                     Proj1 proj1 = {}, Proj2 proj2 = {}) noexcept {
        return operator()(::std::ranges::begin(r1), ::std::ranges::end(r1),
                          ::std::ranges::begin(r2), ::std::ranges::end(r2),
                          ::std::move(pred),
                          ::std::move(proj1), ::std::move(proj2));
    }
};
// clang-format on

inline constexpr auto unsafe_equal = equal_fn{};

} // namespace detail

namespace ranges {} // namespace ranges

} // namespace flux::fou
