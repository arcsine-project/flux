#pragma once
#include <flux/foundation/memory/allocator_traits.hpp>

namespace flux::fou {

template <typename Allocator>
[[nodiscard]] constexpr auto allocate_at_least(Allocator& allocator, ::std::size_t n) noexcept {
    return allocator_traits<Allocator>::allocate_at_least(allocator, n);
}

} // namespace flux::fou