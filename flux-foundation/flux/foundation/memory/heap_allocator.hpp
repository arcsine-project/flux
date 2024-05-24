#pragma once
#include <flux/foundation/memory/detail/low_level_allocator_adapter.hpp>

#if defined(FLUX_USE_MIMALLOC)
// TODO:
//  Implement `mimalloc` allocator support.
#elif FLUX_TARGET(WINDOWS)
#    include <flux/foundation/memory/detail/win32_heap_allocator.hpp>
namespace flux::fou::detail {
using heap_allocator_impl = win32_heap_allocator;
FLUX_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(heap_allocator_impl, heap_allocator_leak_detector);
} // namespace flux::fou::detail
#else
#    include <flux/foundation/detail/malloc_allocator.hpp>
namespace flux::fou::detail {
using heap_allocator_impl = malloc_allocator;
FLUX_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(heap_allocator_impl, heap_allocator_leak_detector);
} // namespace flux::fou::detail
#endif

namespace flux::fou {
using heap_allocator = detail::low_level_allocator_adapter<detail::heap_allocator_impl>;
} // namespace flux::fou