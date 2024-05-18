#pragma once

namespace flux::meta {

// clang-format off
template <typename InputIterator, typename OutputIterator>
struct [[nodiscard]] is_memcpyable final {
    using T = iter_value_t<OutputIterator>;
    using U = decltype(::std::ranges::iter_move(declval<InputIterator&&>()));

    static constexpr bool value = same_as<T, remove_ref_t<U>> and trivially_copyable<T>;
};

template <typename InputIterator, typename OutputIterator>
inline constexpr bool is_memcpyable_v = is_memcpyable<InputIterator, OutputIterator>::value;

template <typename T>
concept addressable = pointer<T> or has_to_address<T> or has_arrow_operator<T>;

template <typename InputIterator, typename OutputIterator>
concept contiguous = contiguous_iterator<InputIterator >
                 and contiguous_iterator<OutputIterator>;

template <typename InputIterator, typename OutputIterator>
concept memcpyable = contiguous     <InputIterator, OutputIterator>
                 and addressable    <OutputIterator>
                 and is_memcpyable_v<InputIterator, OutputIterator>
                 and not_volatile   <InputIterator, OutputIterator>;
// clang-format on

} // namespace flux::meta