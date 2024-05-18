#pragma once

namespace flux::meta {

// clang-format off
template <typename T>
concept relocatable = move_constructible<T> and destructible<T>;

template <typename T>
concept trivially_relocatable = trivially_copyable<T>;
// clang-format on

template <typename T, typename U>
concept same_trivially_relocatable =
        not_volatile<T, U> and is_same_uncvref_v<T, U> and trivially_relocatable<remove_cvref_t<U>>;

template <typename Src, typename Dest>
concept relocatable_from = nothrow_constructible<Dest, Src> and nothrow_destructible<Src>;

} // namespace flux::meta