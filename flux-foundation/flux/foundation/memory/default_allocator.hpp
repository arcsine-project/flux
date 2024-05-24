#pragma once
#include <flux/foundation/memory/heap_allocator.hpp>
#include <flux/foundation/memory/static_allocator.hpp>

namespace flux::fou {
// The default allocator that will be used as `BlockAllocator` in memory arenas.
// Arena allocators like `memory_stack` or `memory_pool` allocate memory by sub-
// dividing a huge block. They get a block allocator that will be used for their
// internal allocation, this type is the default value.
// The default is Heap_allocator.
using default_allocator = heap_allocator;
} // namespace flux::fou