#pragma once
#include <flux/foundation/memory/allocation_result.hpp>
#include <flux/foundation/memory/debugging.hpp>
#include <flux/foundation/utility/terminate.hpp>

#if defined(FLUX_HAS_C11_ALIGNED_ALLOC)
#    include <stdlib.h>
#elif defined(_WIN64) && !defined(__WINE__)
#    if defined(FLUX_HAS_WIN32_ALIGNED_MALLOC)
#        include <malloc.h>
#    endif
#elif defined(FLUX_HAS_GNU_MALLOC_SIZE)
#    include <malloc.h>
#elif defined(FLUX_HAS_OSX_MALLOC_SIZE)
#    include <malloc/malloc.h>
#    define malloc_usable_size malloc_size
#elif __has_include(<malloc_np.h>)
#    include <malloc_np.h>
#else
#    error "Missing implementation for posix_memalign, memalign or _aligned_malloc"
#endif

namespace flux::fou::detail {

// clang-format off
FLUX_ALWAYS_INLINE
inline auto malloc_usable_size_impl(void* ptr) noexcept {
#ifdef FLUX_HAS_WIN32_ALIGNED_MALLOC
    return noexcept_call(::_msize, ptr);
#else
    return ::malloc_usable_size(ptr);
#endif
}

// Low-level allocator.
struct [[nodiscard]] malloc_allocator final {
    using size_type         = ::std::size_t;
    using difference_type   = ::std::ptrdiff_t;
    using allocation_result = allocation_result<void*, size_type>;

    static inline auto info() noexcept {
        return allocator_info{"flux::fou::detail::malloc_allocator", nullptr};
    }

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#endif
    static inline void* allocate(size_type                  size,
                                 [[maybe_unused]] size_type alignment = 0) noexcept {
        if (0 == size) [[unlikely]] {
            size = 1;
        }

        void* memory =
#if __has_builtin(__builtin_malloc)
                __builtin_malloc(size);
#else
                ::std::malloc(size);
#endif
        if (!memory) [[unlikely]] {
            fast_terminate();
        }

        return memory;
    }

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#endif
    static inline void* reallocate(void* memory, size_type size) noexcept {
        if (0 == size) [[unlikely]] {
            size = 1;
        }

        memory =
#if __has_builtin(__builtin_realloc)
                __builtin_realloc(memory, size);
#else
                ::std::realloc(memory, size);
#endif
        if (!memory) [[unlikely]] {
            fast_terminate();
        }

        return memory;
    }

    static inline void deallocate(void*                      memory,
                                  [[maybe_unused]] size_type size = 0,
                                  [[maybe_unused]] size_type alignment = 0) noexcept {
        if (!memory) {
            return;
        }

#if __has_builtin(__builtin_free)
        __builtin_free(memory);
#else
        ::std::free(memory);
#endif
    }

    static inline allocation_result allocate_at_least(size_type size) noexcept {
        auto memory = allocate(size);
        return {memory, malloc_usable_size_impl(memory)};
    }

    static inline allocation_result reallocate_at_least(void* memory, size_type size) noexcept {
        auto new_memory = reallocate(memory, size);
        return {new_memory, malloc_usable_size_impl(new_memory)};
    }

#if FLUX_MEMORY_ENABLE_ALIGNED_ALLOC && defined(FLUX_HAS_WIN32_ALIGNED_MALLOC)
#    if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#    endif
    static inline void* allocate_aligned(size_type size, size_type alignment) noexcept {
        if (0 == size) [[unlikely]] {
            size = 1;
        }

        void* memory = noexcept_call(::_aligned_malloc, size, alignment);
        if (!memory) [[unlikely]] {
            fast_terminate();
        }

        return memory;
    }

#    if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#    endif
    static inline void* reallocate_aligned(void*     memory,
                                           size_type size,
                                           size_type alignment) noexcept {
        if (0 == size) [[unlikely]] {
            size = 1;
        }

        memory = noexcept_call(::_aligned_realloc, memory, size, alignment);
        if (!memory) [[unlikely]] {
            fast_terminate();
        }

        return memory;
    }

    static inline void deallocate_aligned(void*                      memory,
                                          [[maybe_unused]] size_type alignment = 0) noexcept {
        if (!memory) {
            return;
        }

        noexcept_call(::_aligned_free, memory);
    }

    static inline allocation_result allocate_aligned_at_least(size_type size,
                                                              size_type alignment) noexcept {
        auto memory = allocate_aligned(size, alignment);
        return {memory, noexcept_call(::_aligned_msize, memory, alignment, 0ull)};
    }

    static inline allocation_result reallocate_aligned_at_least(void*     memory,
                                                                size_type size,
                                                                size_type alignment) noexcept {
        auto new_memory = reallocate_aligned(memory, size, alignment);
        return {new_memory, noexcept_call(::_aligned_msize, new_memory, alignment, 0ull)};
    }
#endif

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return size_type(-1) / sizeof(::std::byte);
    }
};
// clang-format on

} // namespace flux::fou::detail