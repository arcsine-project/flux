#pragma once
#include <flux/meta.hpp>

#include <flux/foundation/memory/detail/constexpr_memcpy.hpp>
#include <flux/foundation/utility/detail/to_address.hpp>

namespace flux::fou {

namespace detail {

// clang-format off
struct [[nodiscard]] move_fn final {
    template <meta::input_iterator              InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::nothrow_forward_iterator    OutputIterator>
    requires (meta::iter_move_constructible<OutputIterator, InputIterator>)
    FLUX_ALWAYS_INLINE
    static constexpr OutputIterator operator()(InputIterator  first,
                                               InputSentinel  last,
                                               OutputIterator result) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            auto const count = last - first;
            detail::constexpr_memmove(detail::to_address(result), detail::to_address(first),
                                      (::std::size_t)count);
            return result + count;
        } else {
            for (; first != last; ++result, (void)++first) {
                *result = ::std::move(*first);
            }
            return result;
        }
    }
};
// clang-format on

inline constexpr auto unsafe_move = move_fn{};

} // namespace detail

namespace ranges {} // namespace ranges

} // namespace flux::fou
