#pragma once

namespace flux::meta {

using ::std::assignable_from;
using ::std::bidirectional_iterator;
using ::std::common_reference_with;
using ::std::constructible_from;
using ::std::contiguous_iterator;
using ::std::convertible_to;
using ::std::default_initializable;
using ::std::derived_from;
using ::std::forward_iterator;
using ::std::indirectly_copyable;
using ::std::indirectly_movable;
using ::std::input_iterator;
using ::std::input_or_output_iterator;
using ::std::integral;
using ::std::movable;
using ::std::predicate;
using ::std::random_access_iterator;
using ::std::same_as;
using ::std::sentinel_for;
using ::std::sized_sentinel_for;
using ::std::ranges::input_range;
using ::std::ranges::range;

template <typename T, ::std::size_t Size>
concept same_size = requires { requires sizeof(T) == Size; };

template <typename T, ::std::size_t Alignment>
concept same_align = requires { requires alignof(T) == Alignment; };

template <typename... Ts>
concept distinct = are_distinct_v<Ts...>;

template <typename T, typename... Ts>
concept contains = is_contains_v<T, Ts...>;

template <typename To, typename From>
concept assignable = ::std::is_assignable_v<To, From>;

template <typename T, typename... Ts>
concept constructible = ::std::is_constructible_v<T, Ts...>;

template <typename T, typename... Ts>
concept nothrow_constructible = ::std::is_nothrow_constructible_v<T, Ts...>;
template <typename T>
concept nothrow_destructible = ::std::is_nothrow_destructible_v<T>;

template <typename T>
concept trivial = ::std::is_trivial_v<T>;

template <typename T>
concept standard_layout = ::std::is_standard_layout_v<T>;

template <typename T>
concept default_constructible = ::std::is_default_constructible_v<T>;

template <typename T>
concept trivially_constructible = ::std::is_trivially_constructible_v<T>;

template <typename T>
concept trivially_default_constructible = ::std::is_trivially_default_constructible_v<T>;

template <typename T>
concept trivially_copyable = ::std::is_trivially_copyable_v<T>;

template <typename T>
concept trivially_copy_constructible = ::std::is_trivially_copy_constructible_v<T>;
template <typename T>
concept trivially_move_constructible = ::std::is_trivially_move_constructible_v<T>;

template <typename T>
concept copy_assignable = ::std::is_copy_assignable_v<T>;
template <typename T>
concept move_assignable = ::std::is_move_assignable_v<T>;

template <typename T>
concept trivially_copy_assignable = ::std::is_trivially_copy_assignable_v<T>;
template <typename T>
concept trivially_move_assignable = ::std::is_trivially_move_assignable_v<T>;

template <typename T>
concept trivially_destructible = ::std::is_trivially_destructible_v<T>;
template <typename T>
concept not_trivially_destructible = not trivially_destructible<T>;
template <typename T>
concept destructible = ::std::is_destructible_v<T>;

template <typename T>
concept has_trivial_lifetime = trivially_default_constructible<T> and trivially_destructible<T>;

template <typename T>
concept nothrow_copy_constructible = ::std::is_nothrow_copy_constructible_v<T>;
template <typename T>
concept nothrow_move_constructible = ::std::is_nothrow_move_constructible_v<T>;

template <typename T>
concept nothrow_copy_assignable = ::std::is_nothrow_copy_assignable_v<T>;
template <typename T>
concept nothrow_move_assignable = ::std::is_nothrow_move_assignable_v<T>;

template <typename T>
concept copy_constructible = ::std::copy_constructible<T>;
template <typename T>
concept move_constructible = ::std::move_constructible<T>;

template <typename T, typename U>
concept trivially_lexicographically_comparable =
        same_as<remove_cv_t<T>, remove_cv_t<U>> and sizeof(T) == 1 and ::std::is_unsigned_v<T>;

template <typename T, typename U>
concept trivially_equality_comparable = is_trivially_equality_comparable_v<T, U>;

namespace detail {
template <typename T>
concept boolean_testable_impl = convertible_to<T, bool>;
} // namespace detail

template <typename T>
concept boolean_testable = detail::boolean_testable_impl<T> and requires(T&& t) {
    { not forward<T>(t) } -> detail::boolean_testable_impl;
};

template <typename T>
concept abstract = ::std::is_abstract_v<T>;
template <typename T>
concept not_abstract = not abstract<T>;

template <typename T>
concept empty = meta::is_empty_v<T>;
template <typename T>
concept object = ::std::is_object_v<T>;
template <typename T>
concept function = ::std::is_function_v<T>;
template <typename T>
concept member_function = ::std::is_member_function_pointer_v<T>;

template <typename T>
concept reference = ::std::is_reference_v<T>;
template <typename T>
concept not_reference = not reference<T>;
template <typename T>
concept pointer = ::std::is_pointer_v<T>;
template <typename T>
concept not_pointer = not pointer<T>;

template <typename T>
concept non_cv = same_as<::std::remove_cv_t<T>, T>;

template <typename T>
concept integer = ::std::integral<T> and not same_as<remove_cvref_t<T>, bool>;
template <typename T>
concept unsigned_integer = ::std::unsigned_integral<T> and not same_as<remove_cvref_t<T>, bool>;

template <typename... Ts>
concept not_volatile = (not ::std::is_volatile_v<Ts> and ...);

template <typename T, typename... Args>
concept underlying_constructible = ::std::conjunction_v<is_constructible_from<T, Args>...>;

template <typename T, typename... Args>
concept only_constructible = requires(Args&&... args) { new T{::std::forward<Args>(args)...}; };

// This concept ensures that uninitialized algorithms can construct an object
// at the address pointed-to by the iterator, which requires an lvalue.
template <typename Iterator>
concept nothrow_input_iterator =
        input_iterator<Iterator> and is_lvalue_reference_v<iter_ref_t<Iterator>> and
        same_as<remove_cvref_t<iter_ref_t<Iterator>>, remove_ref_t<iter_ref_t<Iterator>>> and
        same_as<remove_cvref_t<iter_ref_t<Iterator>>, iter_value_t<Iterator>>;

template <typename Sentinel, typename Iterator>
concept nothrow_sentinel_for = sentinel_for<Sentinel, Iterator>;

template <typename Iterator>
concept nothrow_forward_iterator =
        nothrow_input_iterator<Iterator> and forward_iterator<Iterator> and
        nothrow_sentinel_for<Iterator, Iterator>;

template <typename Range>
concept nothrow_input_range = range<Range> and nothrow_input_iterator<iterator_t<Range>> and
                              nothrow_sentinel_for<sentinel_t<Range>, iterator_t<Range>>;

template <typename Range>
concept nothrow_forward_range =
        nothrow_input_range<Range> and nothrow_forward_iterator<iterator_t<Range>>;

template <typename NoThrowForwardIterator>
concept use_memset_value_construct = contiguous_iterator<NoThrowForwardIterator> and
                                     trivially_copyable<iter_value_t<NoThrowForwardIterator>> and
                                     not_volatile<remove_ref_t<iter_ref_t<NoThrowForwardIterator>>>;

template <typename T>
concept has_to_address = requires(T const p) { p.to_address(); } or
                         requires(T const p) { ::std::pointer_traits<T>::to_address(p); };
template <typename T>
concept has_arrow_operator = requires(T const p) { p.operator->(); };

template <typename InputIterator, typename OutputIterator>
concept iter_move_constructible =
        constructible_from<iter_value_t<OutputIterator>, iter_rvref_t<InputIterator>>;

template <typename InputIterator, typename OutputIterator>
concept iter_copy_constructible =
        constructible_from<iter_value_t<OutputIterator>, iter_ref_t<InputIterator>>;

} // namespace flux::meta