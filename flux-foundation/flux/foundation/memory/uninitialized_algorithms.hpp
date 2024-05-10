#pragma once
#include <flux/foundation/memory/construct.hpp>
#include <flux/foundation/memory/relocate.hpp>

namespace flux::fou {

// clang-format off
template <typename T>
constexpr T* uninitialized_default_construct(T* const p) noexcept {
    if consteval {
        return detail::construct_at(p);
    }
    return ::new (p) T;
}

template <typename T>
constexpr T* uninitialized_value_construct(T* const p) noexcept {
    return detail::construct_at(p);
}
// clang-format on

template <typename ForwardIterator>
constexpr void uninitialized_default_construct(ForwardIterator first,
                                               ForwardIterator last) noexcept {
    if consteval {
        for (; first != last; ++first) {
            detail::construct_at(detail::to_address(first));
        }
    } else {
        using value_type = meta::iter_value_t<ForwardIterator>;
        for (; first != last; ++first) {
            ::new (detail::voidify(*first)) value_type;
        }
    }
}

template <typename ForwardIterator, meta::integer Integer>
constexpr void uninitialized_default_construct_n(ForwardIterator first, Integer n) noexcept {
    if consteval {
        for (; n > 0; ++first, (void)--n) {
            detail::construct_at(detail::to_address(first));
        }
    } else {
        using value_type = meta::iter_value_t<ForwardIterator>;
        for (; n > 0; ++first, (void)--n) {
            ::new (detail::voidify(*first)) value_type;
        }
    }
}

template <typename ForwardIterator>
constexpr void uninitialized_value_construct(ForwardIterator first, ForwardIterator last) noexcept {
    for (; first != last; ++first) {
        detail::construct_at(detail::to_address(first));
    }
}

template <typename ForwardIterator, meta::integer Integer>
constexpr void uninitialized_value_construct_n(ForwardIterator first, Integer n) noexcept {
    for (; n > 0; ++first, (void)--n) {
        detail::construct_at(detail::to_address(first));
    }
}

// clang-format off
template <typename ForwardIterator, typename T>
constexpr void uninitialized_construct(ForwardIterator first,
                                       ForwardIterator last,
                                       T const&        value) noexcept {
    for (; first != last; ++first) {
        detail::construct_at(detail::to_address(first), value);
    }
}
template <typename ForwardIterator, typename T>
constexpr void uninitialized_construct(ForwardIterator first,
                                       ForwardIterator last,
                                       T&&             value) noexcept {
    for (; first != last; ++first) {
        detail::construct_at(detail::to_address(first), ::std::move(value));
    }
}

template <typename ForwardIterator, meta::integer Integer, typename T>
constexpr void uninitialized_construct_n(ForwardIterator first,
                                         Integer         count,
                                         T const&        value) noexcept {
    for (; count > 0; ++first, (void)--count) {
        detail::construct_at(detail::to_address(first), value);
    }
}
template <typename ForwardIterator, meta::integer Integer, typename T>
constexpr void uninitialized_construct_n(ForwardIterator first,
                                         Integer         count,
                                         T&&             value) noexcept {
    for (; count > 0; ++first, (void)--count) {
        detail::construct_at(detail::to_address(first), ::std::move(value));
    }
}

template <typename InputIterator, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<ForwardIterator>>
constexpr void uninitialized_move(InputIterator   first,
                                  InputIterator   last,
                                  ForwardIterator result) noexcept {
    auto current = result;
    for (; first != last; (void)++current, ++first) {
        detail::construct_at(detail::to_address(current), ::std::move(*first));
    }
}

template <typename InputIterator, meta::integer Integer, typename ForwardIterator>
    requires meta::nothrow_move_constructible<meta::iter_value_t<ForwardIterator>>
constexpr void uninitialized_move_n(InputIterator   first,
                                    Integer         count,
                                    ForwardIterator result) noexcept {
    auto current = result;
    for (; count > 0; (void)++current, ++first, --count) {
        detail::construct_at(detail::to_address(current), ::std::move(*first));
    }
}

template <typename InputIterator, typename ForwardIterator>
constexpr auto uninitialized_copy(InputIterator   first,
                                  InputIterator   last,
                                  ForwardIterator result) noexcept {
    auto current = result;
    for (; first != last; (void)++current, ++first) {
        detail::construct_at(detail::to_address(current), *first);
    }
    return ::std::move(current);
}

template <typename InputIterator, meta::integer Integer, typename ForwardIterator>
constexpr void uninitialized_copy_n(InputIterator   first,
                                    Integer         count,
                                    ForwardIterator result) noexcept {
    auto current = result;
    for (; count > 0; (void)++current, ++first, --count) {
        detail::construct_at(detail::to_address(current), *first);
    }
}

template <typename InputIterator, typename ForwardIterator>
constexpr auto uninitialized_copy_no_overlap(InputIterator   first,
                                             InputIterator   last,
                                             ForwardIterator result) noexcept {
    if constexpr (meta::memcpyable<InputIterator, ForwardIterator>) {
        if consteval {
            return uninitialized_copy(::std::move(first), ::std::move(last), ::std::move(result));
        } else {
            auto const count = static_cast<::std::size_t>(last - first);
            detail::constexpr_memcpy(detail::to_address(result), detail::to_address(first), count);
            return result + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        return uninitialized_copy(::std::move(first), ::std::move(last), ::std::move(result));
    }
}

template <typename InputIterator, typename ForwardIterator>
constexpr auto uninitialized_relocate(InputIterator   first,
                                      InputIterator   last,
                                      ForwardIterator result) noexcept {
    using Src = decltype(::std::ranges::iter_move(first));
    using Dst = meta::iter_value_t<ForwardIterator>;
    static_assert(meta::nothrow_constructible<Dst, Src> and
                          meta::destructible<meta::remove_cvref_t<Src>>,
                  "::new (voidify(*dest)) T(std::move(*src)) must be well-formed");
    if constexpr (
        meta::same_trivially_relocatable<Src, Dst> and
        meta::contiguous<InputIterator, ForwardIterator>
    ) {
        if consteval {
            auto current = result;
            for (; first != last; (void)++current, ++first) {
                relocate_at(detail::to_address(first), detail::to_address(current));
            }
            return current;
        } else {
            auto const count = static_cast<::std::size_t>(last - first);
            detail::constexpr_memmove(detail::to_address(result), detail::to_address(first), count);
            return result + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        auto current = result;
        for (; first != last; (void)++current, ++first) {
            relocate_at(detail::to_address(first), detail::to_address(current));
        }
        return current;
    }
}

template <typename InputIterator, typename ForwardIterator>
constexpr auto uninitialized_relocate_no_overlap(InputIterator   first,
                                                 InputIterator   last,
                                                 ForwardIterator result) noexcept {
    using Src = decltype(::std::ranges::iter_move(first));
    using Dst = meta::iter_value_t<ForwardIterator>;
    static_assert(meta::nothrow_constructible<Dst, Src> and
                          meta::destructible<meta::remove_cvref_t<Src>>,
                  "::new (voidify(*dest)) T(std::move(*src)) must be well-formed");
    if constexpr (
        meta::same_trivially_relocatable<Src, Dst> and
        meta::contiguous<InputIterator, ForwardIterator>
    ) {
        if consteval {
            auto current = result;
            for (; first != last; (void)++current, ++first) {
                relocate_at(detail::to_address(first), detail::to_address(current));
            }
            return current;
        } else {
            auto const count = static_cast<::std::size_t>(last - first);
            detail::constexpr_memcpy(detail::to_address(result), detail::to_address(first), count);
            return result + meta::iter_diff_t<ForwardIterator>(count);
        }
    } else {
        auto current = result;
        for (; first != last; (void)++current, ++first) {
            relocate_at(detail::to_address(first), detail::to_address(current));
        }
        return current;
    }
}
// clang-format on

} // namespace flux::fou