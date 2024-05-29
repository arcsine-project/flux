#pragma once
#include <flux/foundation/memory/construct.hpp>
#include <flux/foundation/memory/relocate.hpp>

namespace flux::fou {

namespace detail {

struct [[nodiscard]] uninitialized_default_construct_fn final {
    template <meta::nothrow_forward_iterator       Iterator,
              meta::nothrow_sentinel_for<Iterator> Sentinel>
        requires meta::default_initializable<meta::iter_value_t<Iterator>>
    static constexpr Iterator operator()(Iterator first, Sentinel last) noexcept {
        using ValueType = meta::remove_ref_t<meta::iter_ref_t<Iterator>>;
        if consteval {
            for (; first != last; ++first) {
                detail::construct_at<ValueType>(detail::to_address(first));
            }
            return first;
        } else {
            if constexpr (meta::trivially_default_constructible<ValueType>) {
                return ranges::next(first, last);
            } else {
                for (; first != last; ++first) {
                    ::new (static_cast<void*>(detail::to_address(first))) ValueType;
                }
                return first;
            }
        }
    }

    template <meta::nothrow_forward_range Range>
        requires meta::default_initializable<meta::range_value_t<Range>>
    static constexpr meta::borrowed_iter_t<Range> operator()(Range&& range) noexcept {
        return operator()(::std::ranges::begin(range), ::std::ranges::end(range));
    }
};

struct [[nodiscard]] uninitialized_default_construct_n_fn final {
    template <meta::nothrow_forward_iterator Iterator>
        requires meta::default_initializable<meta::iter_value_t<Iterator>>
    static constexpr Iterator operator()(Iterator first, meta::iter_diff_t<Iterator> n) noexcept {
        using ValueType = meta::remove_ref_t<meta::iter_ref_t<Iterator>>;
        if consteval {
            for (; n > 0; ++first, (void)--n) {
                detail::construct_at<ValueType>(detail::to_address(first));
            }
            return first;
        } else {
            if constexpr (meta::trivially_default_constructible<ValueType>) {
                return ranges::next(first, n);
            } else {
                for (; n > 0; ++first, (void)--n) {
                    ::new (static_cast<void*>(detail::to_address(first))) ValueType;
                }
                return first;
            }
        }
    }
};

struct [[nodiscard]] uninitialized_value_construct_fn final {
    template <meta::nothrow_forward_iterator       Iterator,
              meta::nothrow_sentinel_for<Iterator> Sentinel>
        requires meta::default_initializable<meta::iter_value_t<Iterator>>
    static constexpr Iterator operator()(Iterator first, Sentinel last) noexcept {
        using ValueType = meta::remove_ref_t<meta::iter_ref_t<Iterator>>;
        if constexpr (meta::use_memset_value_construct<Iterator>) {
            if not consteval {
                char* const first_byte = (char*)detail::to_address(first);
                auto const  n_bytes    = (char*)detail::to_address(last) - first_byte;
                __builtin_memset((void*)first_byte, 0, (std::size_t)n_bytes);
                return last;
            }
        }

        for (; first != last; ++first) {
            detail::construct_at<ValueType>(detail::to_address(first));
        }
        return first;
    }

    template <meta::nothrow_forward_range Range>
        requires meta::default_initializable<meta::range_value_t<Range>>
    static constexpr meta::borrowed_iter_t<Range> operator()(Range&& range) noexcept {
        return operator()(::std::ranges::begin(range), ::std::ranges::end(range));
    }
};

struct [[nodiscard]] uninitialized_value_construct_n_fn final {
    template <meta::nothrow_forward_iterator Iterator>
        requires meta::default_initializable<meta::iter_value_t<Iterator>>
    static constexpr Iterator operator()(Iterator first, meta::iter_diff_t<Iterator> n) noexcept {
        using ValueType = meta::remove_ref_t<meta::iter_ref_t<Iterator>>;
        if constexpr (meta::use_memset_value_construct<Iterator>) {
            if not consteval {
                auto* const first_byte = (char*)detail::to_address(first);
                auto const  n_bytes    = sizeof(ValueType) * (std::size_t)n;
                __builtin_memset((void*)first_byte, 0, n_bytes);
                return first + n;
            }
        }

        for (; n > 0; ++first, (void)--n) {
            detail::construct_at<ValueType>(detail::to_address(first));
        }
        return first;
    }
};

struct [[nodiscard]] uninitialized_construct_n_fn final {
    template <meta::nothrow_forward_iterator Iterator, typename T>
        requires meta::constructible_from<meta::iter_value_t<Iterator>, T>
    static constexpr Iterator operator()(Iterator first, meta::iter_diff_t<Iterator> n,
                                         T const& value) noexcept {
        using ValueType = meta::remove_ref_t<meta::iter_ref_t<Iterator>>;
        if constexpr (meta::use_memset_value_construct<Iterator> and meta::is_byte<T>) {
            if not consteval {
                auto* const first_byte = (char*)detail::to_address(first);
                auto const  n_bytes    = sizeof(ValueType) * (std::size_t)n;
                __builtin_memset((void*)first_byte, value, n_bytes);
                return first + n;
            }
        }

        for (; n > 0; ++first, (void)--n) {
            detail::construct_at<ValueType>(detail::to_address(first), value);
        }
        return first;
    }
};

// clang-format off
template <bool IsMove>
struct [[nodiscard]] uninitialized_memcpy_or_memmove_fn final {
    template <typename InputIterator, typename OutputIterator>
    using result = in_out_result<InputIterator, OutputIterator>;

    template <meta::input_iterator              InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::nothrow_forward_iterator    OutputIterator>
        requires(IsMove ? meta::iter_move_constructible<OutputIterator, InputIterator>
                        : meta::iter_copy_constructible<OutputIterator, InputIterator>)
    FLUX_ALWAYS_INLINE
    static constexpr OutputIterator operator()(InputIterator  first,
                                               InputSentinel  last,
                                               OutputIterator result) noexcept {
        using OutType = meta::remove_ref_t<meta::iter_ref_t<OutputIterator>>;
        static_assert(IsMove ? meta::nothrow_move_assignable<OutType>
                             : meta::nothrow_copy_assignable<OutType>);
        if constexpr (IsMove) {
            auto const count = last - first;
            detail::constexpr_memmove(detail::to_address(result), detail::to_address(first),
                                      (std::size_t)count);
            return result + count;
        } else {
            auto const count = last - first;
            detail::constexpr_memcpy(detail::to_address(result), detail::to_address(first),
                                     (std::size_t)count);
            return result + count;
        }
    }

    template <meta::input_iterator                       InputIterator,
              meta::sentinel_for<InputIterator>          InputSentinel,
              meta::nothrow_forward_iterator             OutputIterator,
              meta::nothrow_sentinel_for<OutputIterator> OutputSentinel>
        requires(IsMove ? meta::iter_move_constructible<OutputIterator, InputIterator>
                        : meta::iter_copy_constructible<OutputIterator, InputIterator>)
    FLUX_ALWAYS_INLINE
    static constexpr auto operator()(InputIterator  ifirst, InputSentinel  ilast,
                                     OutputIterator ofirst, OutputSentinel olast) noexcept
            -> result<InputIterator, OutputIterator> {
        using OutType = meta::remove_ref_t<meta::iter_ref_t<OutputIterator>>;
        static_assert(IsMove ? meta::nothrow_move_constructible<OutType>
                             : meta::nothrow_copy_constructible<OutType>);
        constexpr bool sized_input  = meta::sized_sentinel_for< InputSentinel,  InputIterator>;
        constexpr bool sized_output = meta::sized_sentinel_for<OutputSentinel, OutputIterator>;
        if constexpr (IsMove) {
            if constexpr (sized_input && sized_output) {
                auto const count = ::std::min(ilast - ifirst, olast - ofirst);
                detail::constexpr_memmove(detail::to_address(ofirst), detail::to_address(ifirst),
                                          (std::size_t)count);
                return {ifirst + count, ofirst + count};
            } else if constexpr (sized_input) {
                auto const count = ilast - ifirst;
                detail::constexpr_memmove(detail::to_address(ofirst), detail::to_address(ifirst),
                                          (std::size_t)count);
                return {ifirst + count, ofirst + count};
            } else if constexpr (sized_output) {
                auto const count = olast - ofirst;
                detail::constexpr_memmove(detail::to_address(ofirst), detail::to_address(ifirst),
                                          (std::size_t)count);
                return {ifirst + count, ofirst + count};
            } else {
                static_assert(false, "two ranges with unreachable sentinels");
            }
        } else {
            if constexpr (sized_input && sized_output) {
                auto const count = ::std::min(ilast - ifirst, olast - ofirst);
                detail::constexpr_memcpy(detail::to_address(ofirst), detail::to_address(ifirst),
                                         (std::size_t)count);
                return {ifirst + count, ofirst + count};
            } else if constexpr (sized_input) {
                auto const count = ilast - ifirst;
                detail::constexpr_memcpy(detail::to_address(ofirst), detail::to_address(ifirst),
                                         (std::size_t)count);
                return {ifirst + count, ofirst + count};
            } else if constexpr (sized_output) {
                auto const count = olast - ofirst;
                detail::constexpr_memcpy(detail::to_address(ofirst), detail::to_address(ifirst),
                                         (std::size_t)count);
                return {ifirst + count, ofirst + count};
            } else {
                static_assert(false, "two ranges with unreachable sentinels");
            }
        }
    }
};
inline constexpr auto uninitialized_memcpy__  = uninitialized_memcpy_or_memmove_fn<false>{};
inline constexpr auto uninitialized_memmove__ = uninitialized_memcpy_or_memmove_fn<true>{};

template <bool IsMove>
struct [[nodiscard]] uninitialized_copy_or_move_fn final {
    template <typename InputIterator, typename OutputIterator>
    using result = in_out_result<InputIterator, OutputIterator>;

    template <meta::input_iterator              InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::nothrow_forward_iterator    OutputIterator>
        requires(IsMove ? meta::iter_move_constructible<OutputIterator, InputIterator>
                        : meta::iter_copy_constructible<OutputIterator, InputIterator>)
    FLUX_ALWAYS_INLINE
    static constexpr OutputIterator operator()(InputIterator  first,
                                               InputSentinel  last,
                                               OutputIterator result) noexcept {
        using OutType = meta::remove_ref_t<meta::iter_ref_t<OutputIterator>>;
        auto current = result;
        for (; first != last; ++current, (void)++first) {
            if constexpr (IsMove) {
                detail::construct_at<OutType>(detail::to_address(current), ::std::move(*first));
            } else {
                detail::construct_at<OutType>(detail::to_address(current), *first);
            }
        }
        return current;
    }

    template <meta::input_iterator                       InputIterator,
              meta::sentinel_for<InputIterator>          InputSentinel,
              meta::nothrow_forward_iterator             OutputIterator,
              meta::nothrow_sentinel_for<OutputIterator> OutputSentinel>
        requires(IsMove ? meta::iter_move_constructible<OutputIterator, InputIterator>
                        : meta::iter_copy_constructible<OutputIterator, InputIterator>)
    FLUX_ALWAYS_INLINE
    static constexpr auto operator()(InputIterator  ifirst, InputSentinel  ilast,
                                     OutputIterator ofirst, OutputSentinel olast) noexcept
            -> result<InputIterator, OutputIterator> {
        using OutType = meta::remove_ref_t<meta::iter_ref_t<OutputIterator>>;
        for (; ifirst != ilast && ofirst != olast; ++ofirst, (void)++ifirst) {
            if constexpr (IsMove) {
                detail::construct_at<OutType>(detail::to_address(ofirst), ::std::move(*ifirst));
            } else {
                detail::construct_at<OutType>(detail::to_address(ofirst), *ifirst);
            }
        }
        return {::std::move(ifirst), ::std::move(ofirst)};
    }
};
inline constexpr auto uninitialized_copy__ = uninitialized_copy_or_move_fn<false>{};
inline constexpr auto uninitialized_move__ = uninitialized_copy_or_move_fn<true>{};

template <bool IsMove>
struct [[nodiscard]] uninitialized_copy_or_move_n_fn final {
    template <meta::input_iterator InputIterator, meta::nothrow_forward_iterator OutputIterator>
        requires(IsMove ? meta::iter_move_constructible<OutputIterator, InputIterator>
                        : meta::iter_copy_constructible<OutputIterator, InputIterator>)
    FLUX_ALWAYS_INLINE
    static constexpr OutputIterator operator()(InputIterator                    first,
                                               meta::iter_diff_t<InputIterator> count,
                                               OutputIterator                   result) noexcept {
        using OutType = meta::remove_ref_t<meta::iter_ref_t<OutputIterator>>;
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            static_assert(IsMove ? meta::move_assignable<OutType>
                                 : meta::copy_assignable<OutType>);
            detail::constexpr_memmove(detail::to_address(result), detail::to_address(first),
                                      (std::size_t)count);
            return result + count;
        } else {
            auto current = result;
            for (; count > 0; ++current, (void)++first, (void)--count) {
                if constexpr (IsMove) {
                    detail::construct_at<OutType>(detail::to_address(current), ::std::move(*first));
                } else {
                    detail::construct_at<OutType>(detail::to_address(current), *first);
                }
            }
            return current;
        }
    }
};
inline constexpr auto uninitialized_copy_n__ = uninitialized_copy_or_move_n_fn<false>{};
inline constexpr auto uninitialized_move_n__ = uninitialized_copy_or_move_n_fn<true>{};
// clang-format on

struct [[nodiscard]] uninitialized_copy_fn final {
    template <typename InputIterator, typename OutputIterator>
    using result = in_out_result<InputIterator, OutputIterator>;

    // clang-format off
    template <meta::input_iterator              InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::nothrow_forward_iterator    OutputIterator>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_ref_t  <InputIterator>>
    static constexpr OutputIterator operator()(InputIterator  first,
                                               InputSentinel  last,
                                               OutputIterator result) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memmove__(::std::move(first), ::std::move(last),
                                           ::std::move(result));
        } else {
            return uninitialized_copy__(::std::move(first), ::std::move(last),
                                        ::std::move(result));
        }
    }

    template <meta::input_iterator                       InputIterator,
              meta::sentinel_for<InputIterator>          InputSentinel,
              meta::nothrow_forward_iterator             OutputIterator,
              meta::nothrow_sentinel_for<OutputIterator> OutputSentinel>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_ref_t  <InputIterator>>
    static constexpr auto operator()(InputIterator  ifirst, InputSentinel  ilast,
                                     OutputIterator ofirst, OutputSentinel olast) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memmove__(::std::move(ifirst), ::std::move(ilast),
                                           ::std::move(ofirst), ::std::move(olast));
        } else {
            return uninitialized_copy__(::std::move(ifirst), ::std::move(ilast),
                                        ::std::move(ofirst), ::std::move(olast));
        }
    }

    template <meta::input_range InputRange, meta::nothrow_forward_range OutputRange>
        requires meta::constructible_from<meta::range_value_t<OutputRange>,
                                          meta::range_ref_t  <InputRange>>
    static constexpr auto operator()(InputRange&& in, OutputRange&& out) noexcept
            -> result<meta::borrowed_iter_t<InputRange>, meta::borrowed_iter_t<OutputRange>> {
        return operator()(::std::ranges::begin(in ), ::std::ranges::end(in ),
                          ::std::ranges::begin(out), ::std::ranges::end(out));
    }
    // clang-format on
};

struct [[nodiscard]] uninitialized_copy_n_fn final {
    // clang-format off
    template <meta::input_iterator InputIterator, meta::nothrow_forward_iterator OutputIterator>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_ref_t  <InputIterator>>
    static constexpr OutputIterator operator()(InputIterator                    first,
                                               meta::iter_diff_t<InputIterator> count,
                                               OutputIterator                   result) noexcept {
        return uninitialized_copy_n__(::std::move(first), count, ::std::move(result));
    }
    // clang-format on
};

struct [[nodiscard]] uninitialized_move_fn final {
    template <typename InputIterator, typename OutputIterator>
    using result = in_out_result<InputIterator, OutputIterator>;

    // clang-format off
    template <meta::input_iterator              InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::nothrow_forward_iterator    OutputIterator>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_rvref_t<InputIterator>>
    static constexpr OutputIterator operator()(InputIterator  first,
                                               InputSentinel  last,
                                               OutputIterator result) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memmove__(::std::move(first), ::std::move(last),
                                           ::std::move(result));
        } else {
            return uninitialized_move__(::std::move(first), ::std::move(last),
                                        ::std::move(result));
        }
    }

    template <meta::input_iterator                       InputIterator,
              meta::sentinel_for<InputIterator>          InputSentinel,
              meta::nothrow_forward_iterator             OutputIterator,
              meta::nothrow_sentinel_for<OutputIterator> OutputSentinel>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_rvref_t<InputIterator>>
    static constexpr auto operator()(InputIterator  ifirst, InputSentinel  ilast,
                                     OutputIterator ofirst, OutputSentinel olast) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memmove__(::std::move(ifirst), ::std::move(ilast),
                                           ::std::move(ofirst), ::std::move(olast));
        } else {
            return uninitialized_move__(::std::move(ifirst), ::std::move(ilast),
                                        ::std::move(ofirst), ::std::move(olast));
        }
    }

    template <meta::input_range InputRange, meta::nothrow_forward_range OutputRange>
        requires meta::constructible_from<meta::range_value_t<OutputRange>,
                                          meta::range_rvref_t<InputRange>>
    static constexpr auto operator()(InputRange&& in, OutputRange&& out) noexcept
            -> result<meta::borrowed_iter_t<InputRange>, meta::borrowed_iter_t<OutputRange>> {
        return operator()(::std::ranges::begin(in ), ::std::ranges::end(in ),
                          ::std::ranges::begin(out), ::std::ranges::end(out));
    }
    // clang-format on
};

struct [[nodiscard]] uninitialized_move_n_fn final {
    template <meta::input_iterator InputIterator, meta::nothrow_forward_iterator OutputIterator>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_rvref_t<InputIterator>>
    static constexpr OutputIterator operator()(InputIterator                    first,
                                               meta::iter_diff_t<InputIterator> count,
                                               OutputIterator                   result) noexcept {
        return uninitialized_move_n__(::std::move(first), count, ::std::move(result));
    }
};

struct [[nodiscard]] uninitialized_copy_no_overlap_fn final {
    template <typename InputIterator, typename OutputIterator>
    using result = in_out_result<InputIterator, OutputIterator>;

    // clang-format off
    template <meta::input_iterator              InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::nothrow_forward_iterator    OutputIterator>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_ref_t  <InputIterator>>
    static constexpr OutputIterator operator()(InputIterator  first,
                                               InputSentinel  last,
                                               OutputIterator result) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memcpy__(::std::move(first), ::std::move(last),
                                          ::std::move(result));
        } else {
            return uninitialized_copy__(::std::move(first), ::std::move(last),
                                        ::std::move(result));
        }
    }

    template <meta::input_iterator                       InputIterator,
              meta::sentinel_for<InputIterator>          InputSentinel,
              meta::nothrow_forward_iterator             OutputIterator,
              meta::nothrow_sentinel_for<OutputIterator> OutputSentinel>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_ref_t  <InputIterator>>
    static constexpr auto operator()(InputIterator  ifirst, InputSentinel  ilast,
                                     OutputIterator ofirst, OutputSentinel olast) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memcpy__(::std::move(ifirst), ::std::move(ilast),
                                          ::std::move(ofirst), ::std::move(olast));
        } else {
            return uninitialized_copy__(::std::move(ifirst), ::std::move(ilast),
                                        ::std::move(ofirst), ::std::move(olast));
        }
    }

    template <meta::input_range InputRange, meta::nothrow_forward_range OutputRange>
        requires meta::constructible_from<meta::range_value_t<OutputRange>,
                                          meta::range_ref_t  <InputRange>>
    static constexpr auto operator()(InputRange&& in, OutputRange&& out) noexcept
            -> result<meta::borrowed_iter_t<InputRange>, meta::borrowed_iter_t<OutputRange>> {
        return operator()(::std::ranges::begin(in ), ::std::ranges::end(in ),
                          ::std::ranges::begin(out), ::std::ranges::end(out));
    }
    // clang-format on
};

struct [[nodiscard]] uninitialized_relocate_fn final {
    template <typename InputIterator, typename OutputIterator>
    using result = in_out_result<InputIterator, OutputIterator>;

    // clang-format off
    template <meta::input_iterator              InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::nothrow_forward_iterator    OutputIterator>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_rvref_t<InputIterator>>
    static constexpr OutputIterator operator()(InputIterator  first,
                                               InputSentinel  last,
                                               OutputIterator result) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memmove__(::std::move(first), ::std::move(last),
                                           ::std::move(result));
        } else {
            auto current = result;
            for (; first != last; ++current, (void)++first) {
                relocate_at(detail::to_address(first), detail::to_address(current));
            }
            return current;
        }
    }

    template <meta::input_iterator                       InputIterator,
              meta::sentinel_for<InputIterator>          InputSentinel,
              meta::nothrow_forward_iterator             OutputIterator,
              meta::nothrow_sentinel_for<OutputIterator> OutputSentinel>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_rvref_t<InputIterator>>
    static constexpr auto operator()(InputIterator  ifirst, InputSentinel  ilast,
                                     OutputIterator ofirst, OutputSentinel olast) noexcept {
        using ReturnType = result<InputIterator, OutputIterator>;
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memmove__(::std::move(ifirst), ::std::move(ilast),
                                           ::std::move(ofirst), ::std::move(olast));
        } else {
            for (; ifirst != ilast && ofirst != olast; ++ofirst, (void)++ifirst) {
                relocate_at(detail::to_address(ifirst), detail::to_address(ofirst));
            }
            return ReturnType{::std::move(ifirst), ::std::move(ofirst)};
        }
    }

    template <meta::input_range InputRange, meta::nothrow_forward_range OutputRange>
        requires meta::constructible_from<meta::range_value_t<OutputRange>,
                                          meta::range_rvref_t<InputRange>>
    static constexpr auto operator()(InputRange&& in, OutputRange&& out) noexcept
            -> result<meta::borrowed_iter_t<InputRange>, meta::borrowed_iter_t<OutputRange>> {
        return operator()(::std::ranges::begin(in ), ::std::ranges::end(in ),
                          ::std::ranges::begin(out), ::std::ranges::end(out));
    }
    // clang-format on
};

struct [[nodiscard]] uninitialized_relocate_no_overlap_fn final {
    template <typename InputIterator, typename OutputIterator>
    using result = in_out_result<InputIterator, OutputIterator>;

    // clang-format off
    template <meta::input_iterator              InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::nothrow_forward_iterator    OutputIterator>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_rvref_t<InputIterator>>
    static constexpr OutputIterator operator()(InputIterator  first,
                                               InputSentinel  last,
                                               OutputIterator result) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memcpy__(::std::move(first), ::std::move(last),
                                          ::std::move(result));
        } else {
            auto current = result;
            for (; first != last; ++current, (void)++first) {
                relocate_at(detail::to_address(first), detail::to_address(current));
            }
            return current;
        }
    }

    template <meta::input_iterator                       InputIterator,
              meta::sentinel_for<InputIterator>          InputSentinel,
              meta::nothrow_forward_iterator             OutputIterator,
              meta::nothrow_sentinel_for<OutputIterator> OutputSentinel>
        requires meta::constructible_from<meta::iter_value_t<OutputIterator>,
                                          meta::iter_rvref_t<InputIterator>>
    static constexpr auto operator()(InputIterator  ifirst, InputSentinel  ilast,
                                     OutputIterator ofirst, OutputSentinel olast) noexcept {
        using ReturnType = result<InputIterator, OutputIterator>;
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            return uninitialized_memcpy__(::std::move(ifirst), ::std::move(ilast),
                                          ::std::move(ofirst), ::std::move(olast));
        } else {
            for (; ifirst != ilast && ofirst != olast; ++ofirst, (void)++ifirst) {
                relocate_at(detail::to_address(ifirst), detail::to_address(ofirst));
            }
            return ReturnType{::std::move(ifirst), ::std::move(ofirst)};
        }
    }

    template <meta::input_range InputRange, meta::nothrow_forward_range OutputRange>
        requires meta::constructible_from<meta::range_value_t<OutputRange>,
                                          meta::range_rvref_t<InputRange>>
    static constexpr auto operator()(InputRange&& in, OutputRange&& out) noexcept
            -> result<meta::borrowed_iter_t<InputRange>, meta::borrowed_iter_t<OutputRange>> {
        return operator()(::std::ranges::begin(in ), ::std::ranges::end(in ),
                          ::std::ranges::begin(out), ::std::ranges::end(out));
    }
    // clang-format on
};

struct [[nodiscard]] uninitialized_relocate_backward_fn final {
    template <typename InputIterator, typename OutputIterator>
    using relocate_backward_result = in_out_result<InputIterator, OutputIterator>;

    // clang-format off
    template <meta::bidirectional_iterator      InputIterator,
              meta::sentinel_for<InputIterator> InputSentinel,
              meta::bidirectional_iterator      OutputIterator>
        requires meta::iter_move_constructible<OutputIterator, InputIterator>
    static constexpr relocate_backward_result<InputIterator, OutputIterator>
    operator()(InputIterator first, InputSentinel last, OutputIterator result) noexcept {
        if constexpr (meta::memcpyable<InputIterator, OutputIterator>) {
            if constexpr (meta::sized_sentinel_for<InputSentinel, InputIterator>) {
                auto const count = last - first;
                if (0 != count) {
                    result -= count;
                    detail::constexpr_memmove(detail::to_address(result), detail::to_address(first),
                                              (std::size_t)count);
                }
                return {first + count, result};
            } else {
                static_assert(false, "unreachable sentinel");
            }
        } else {
            auto last_iter          = ranges::next(first, last);
            auto original_last_iter = last_iter;

            while (first != last_iter) {
                --last_iter;
                --result;
                relocate_at(detail::to_address(last_iter), detail::to_address(result));
            }
            return {::std::move(original_last_iter), ::std::move(result)};
        }
    }

    template <meta::bidirectional_range Range, meta::bidirectional_iterator Iterator>
        requires meta::iter_move_constructible<meta::iterator_t<Range>, Iterator>
    static constexpr auto operator()(Range&& range, Iterator&& result) noexcept
            -> relocate_backward_result<meta::borrowed_iter_t<Range>, Iterator> {
        return operator()(::std::ranges::begin(range), ::std::ranges::end(range),
                          ::std::move(result));
    }
    // clang-format on
};

} // namespace detail

namespace ranges {
// clang-format off
inline constexpr auto uninitialized_default_construct   = detail::uninitialized_default_construct_fn{};
inline constexpr auto uninitialized_default_construct_n = detail::uninitialized_default_construct_n_fn{};
inline constexpr auto uninitialized_value_construct     = detail::uninitialized_value_construct_fn{};
inline constexpr auto uninitialized_value_construct_n   = detail::uninitialized_value_construct_n_fn{};
inline constexpr auto uninitialized_construct_n         = detail::uninitialized_construct_n_fn{};
inline constexpr auto uninitialized_copy                = detail::uninitialized_copy_fn{};
inline constexpr auto uninitialized_copy_n              = detail::uninitialized_copy_n_fn{};
inline constexpr auto uninitialized_move                = detail::uninitialized_move_fn{};
inline constexpr auto uninitialized_move_n              = detail::uninitialized_move_n_fn{};
inline constexpr auto uninitialized_copy_no_overlap     = detail::uninitialized_copy_no_overlap_fn{};
inline constexpr auto uninitialized_relocate            = detail::uninitialized_relocate_fn{};
inline constexpr auto uninitialized_relocate_backward   = detail::uninitialized_relocate_backward_fn{};
inline constexpr auto uninitialized_relocate_no_overlap = detail::uninitialized_relocate_no_overlap_fn{};
// clang-format on
} // namespace ranges

} // namespace flux::fou