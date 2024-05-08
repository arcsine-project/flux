#pragma once

// Whether or not the allocation size will be checked.
#define FLUX_MEMORY_CHECK_ALLOCATION_SIZE (1)

// Whether or not allocated memory will be filled with special values.
#define FLUX_MEMORY_DEBUG_FILL (1)

// The size of the fence memory, it has no effect if FLUX_MEMORY_DEBUG_FILL is disabled.
#define FLUX_MEMORY_DEBUG_FENCE (1)

// Whether or not leak checking is enabled.
#define FLUX_MEMORY_DEBUG_LEAK (1)

// Whether or not the deallocation functions will check for pointers that were never allocated by an
// allocator.
#define FLUX_MEMORY_DEBUG_POINTER (1)

// Whether or not the deallocation functions will check for double free errors.
#define FLUX_MEMORY_DEBUG_DOUBLE_FREE (1)

// Whether or not the `temporary_allocator` will use a `temporary_stack` for its allocation. This
// option controls how and if a global, per-thread instance of it is managed. If 2 it is
// automatically managed and created on-demand, if 1 you need explicit lifetime control through the
// `temporary_stack_initializer` class and if 0 there is no stack created automatically. Mode 2 has
// a slight runtime overhead.
#define FLUX_MEMORY_TEMPORARY_STACK_MODE (2)

#ifdef NDEBUG
#    undef FLUX_MEMORY_CHECK_ALLOCATION_SIZE
#    define FLUX_MEMORY_CHECK_ALLOCATION_SIZE (0)

#    undef FLUX_MEMORY_DEBUG_FILL
#    define FLUX_MEMORY_DEBUG_FILL (0)

#    undef FLUX_MEMORY_DEBUG_FENCE
#    define FLUX_MEMORY_DEBUG_FENCE (0)

#    undef FLUX_MEMORY_DEBUG_LEAK
#    define FLUX_MEMORY_DEBUG_LEAK (0)

#    undef FLUX_MEMORY_DEBUG_POINTER
#    define FLUX_MEMORY_DEBUG_POINTER (0)

#    undef FLUX_MEMORY_DEBUG_DOUBLE_FREE
#    define FLUX_MEMORY_DEBUG_DOUBLE_FREE (0)
#endif