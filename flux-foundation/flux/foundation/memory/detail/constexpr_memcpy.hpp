#pragma once
#include <flux/meta/datasizeof.hpp>

namespace flux::fou::detail {

template <typename T, typename U>
    requires meta::trivially_lexicographically_comparable<T, U>
constexpr int constexpr_memcmp(T const* lhs, U const* rhs, ::std::size_t count = 1) noexcept {
    if consteval {
        if (1 == sizeof(T) && !meta::same_as<T, bool>) {
            return __builtin_memcmp(lhs, rhs, count * sizeof(T));
        }

        // clang-format off
        for (; count != 0; --count, ++lhs, ++rhs) {
            if (*lhs < *rhs) return -1;
            if (*rhs < *lhs) return  1;
        }
        // clang-format on
        return 0;
    }
    return __builtin_memcmp(lhs, rhs, count * sizeof(T));
}

// clang-format off
template <typename T, typename U>
    requires meta::trivially_equality_comparable<T, U>
constexpr bool constexpr_memcmp_equal(T const* lhs, U const* rhs, ::std::size_t count = 1) noexcept {
    if consteval {
        if (1 == sizeof(T) && meta::integer<T>) {
            return __builtin_memcmp(lhs, rhs, count * sizeof(T)) == 0;
        }

        for (; count != 0; --count, ++lhs, ++rhs) {
            if (!(*lhs == *rhs)) {
                return false;
            }
        }
        return true;
    }
    return __builtin_memcmp(lhs, rhs, count * sizeof(T)) == 0;
}
// clang-format on

// This function performs an assignment to an existing, already alive TriviallyCopyable object
// from another TriviallyCopyable object.
//
// It basically works around the fact that TriviallyCopyable objects are not required to be
// syntactically copy/move constructible or copy/move assignable. Technically, only one of the
// four operations is required to be syntactically valid -- but at least one definitely has to
// be valid.
// clang-format off
template <typename T, typename U>
    requires meta::assignable<T&, U const&>
constexpr auto assign_trivially_copyable(T& dest, U const& src) noexcept {
    dest = src;
    return dest;
}

template <typename T, typename U>
    requires (not meta::assignable<T&, U const&> and meta::assignable<T&, U&&>)
constexpr auto assign_trivially_copyable(T& dest, U& src) noexcept {
    dest = static_cast<U&&>(src);
    return dest;
}

template <typename T, typename U>
    requires (not meta::assignable   <T&, U const&> and
              not meta::assignable   <T&, U&&>      and
                  meta::constructible<T&, U const&>)
constexpr auto assign_trivially_copyable(T& dest, U const& src) noexcept {
    construct_at(addressof(dest), src);
    return dest;
}

template <typename T, typename U>
    requires (not meta::assignable   <T&, U const&> and
              not meta::assignable   <T&, U&&>      and
              not meta::constructible<T&, U const&> and
                  meta::constructible<T&, U&&>)
constexpr auto assign_trivially_copyable(T& dest, U& src) noexcept {
    construct_at(addressof(dest), static_cast<U&&>(src));
    return dest;
}
// clang-format on

template <typename T, typename U>
    requires meta::trivially_copyable<meta::remove_cv_t<U>>
constexpr auto constexpr_memcpy(T* dest, U const* src, ::std::size_t count = 1) noexcept {
    if consteval {
        if constexpr (meta::same_as<meta::remove_cv_t<T>, meta::remove_cv_t<U>>) {
            __builtin_memcpy(dest, src, count * sizeof(T));
            return dest;
        } else {
            for (::std::size_t i = 0; i != count; ++i) {
                assign_trivially_copyable(dest[i], src[i]);
            }
        }
    } else {
        if (count > 0) {
            __builtin_memcpy(dest, src, (count - 1) * sizeof(T) + meta::data_size_of<T>);
        }
    }
    return dest;
}

template <typename T, typename U>
    requires meta::trivially_copyable<meta::remove_cv_t<U>>
constexpr auto constexpr_memmove(T* dest, U const* src, ::std::size_t count = 1) noexcept {
    if consteval {
        if constexpr (meta::same_as<meta::remove_cv_t<T>, meta::remove_cv_t<U>>) {
            __builtin_memmove(dest, src, count * sizeof(T));
            return dest;
        } else {
            if (is_pointer_in_range(src, src + count, dest)) {
                for (; count > 0; --count) {
                    assign_trivially_copyable(dest[count - 1], src[count - 1]);
                }
            } else {
                for (::std::size_t i = 0; i != count; ++i) {
                    assign_trivially_copyable(dest[i], src[i]);
                }
            }
        }
    } else {
        if (count > 0) {
            __builtin_memmove(dest, src, (count - 1) * sizeof(T) + meta::data_size_of<T>);
        }
    }
    return dest;
}
// clang-format on

} // namespace flux::fou::detail