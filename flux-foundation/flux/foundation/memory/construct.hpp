#pragma once
#include <flux/foundation/memory/detail/construct_at.hpp>
#include <flux/foundation/memory/detail/to_address.hpp>

namespace flux::fou {

template <meta::forward_iterator Iterator, typename T>
constexpr void construct_in_place(Iterator iterator, T const& value) noexcept {
    detail::construct_at(detail::to_address(iterator), value);
}
template <meta::forward_iterator Iterator, typename T>
constexpr void construct_in_place(Iterator iterator, T&& value) noexcept {
    detail::construct_at(detail::to_address(iterator), ::std::move(value));
}
template <meta::forward_iterator Iterator, typename... Args>
constexpr void construct_in_place(Iterator iterator, Args&&... args) noexcept {
    detail::construct_at(detail::to_address(iterator), ::std::forward<Args>(args)...);
}

template <meta::forward_iterator Iterator>
constexpr Iterator destroy_range(Iterator first, Iterator last) noexcept {
    for (; first != last; ++first) {
        detail::destroy_at(addressof(*first));
    }
    return first;
}

template <meta::input_iterator Iterator>
constexpr void destroy_in_place(Iterator iterator) noexcept {
    using value_type = meta::iter_value_t<Iterator>;
    if constexpr (meta::not_trivially_destructible<value_type>) {
        detail::destroy_at(addressof(*iterator));
    }
}

} // namespace flux::fou