#pragma once
#include <flux/config.hpp>

#include <cstddef>

// This trait provides the size of a type excluding any tail padding.
//
// It is useful in contexts where performing an operation using the full size of the class
// (including padding) may have unintended side effects, such as overwriting a derived class' member
// when writing the tail padding of a class through a pointer-to-base.

namespace flux::meta {

// clang-format off
#if __has_extension(datasizeof)
template <typename T>
inline constexpr auto data_size_of = __datasizeof(T);
#else
namespace detail {
// `tail_padding` is sometimes non-standard layout. Using `offsetof` is UB in that case, but GCC and Clang allow
// the use as an extension.
FLUX_DISABLE_WARNING_BLOCK(-Winvalid-offsetof,
template <typename T>
inline constexpr auto data_size_impl = [] constexpr -> ::std::size_t {
    if constexpr (is_empty_v<T>) {
        return 1;
    }

    if constexpr (config::has_no_unique_address) {
        struct tail_padding {
            FLUX_NO_UNIQUE_ADDRESS T value;
            char                     first_padding_byte;
        };
        return offsetof(tail_padding, first_padding_byte);
    } else {
        if constexpr (is_class_v<T> and !is_final_v<T>) {
            struct tail_padding : T {
                char first_padding_byte;
            };
            return offsetof(tail_padding, first_padding_byte);
        } else {
            struct tail_padding {
                T    value;
                char first_padding_byte;
            };
            return offsetof(tail_padding, first_padding_byte);
        }
    }
}();)
} // namespace detail
template <typename T>
inline constexpr auto data_size_of = detail::data_size_impl<T>;
// clang-format on
#endif
} // namespace flux::meta