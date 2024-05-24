#pragma once
#include <flux/foundation/memory/debugging.hpp>
#include <flux/foundation/utility/terminate.hpp>

namespace flux::fou::detail {

// clang-format off
// Low-level allocator.
struct [[nodiscard]] malloc_allocator final {
    using size_type       = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;

    static inline auto info() noexcept {
        return allocator_info{"flux::fou::detail::malloc_allocator", nullptr};
    }

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#endif
    static inline void* allocate(size_type size, size_type) noexcept {
        void* memory = 
#    if __has_builtin(__builtin_malloc)
        __builtin_malloc(size);
#    else
        ::std::malloc(size);
#    endif
        if (!memory) [[unlikely]]
            fast_terminate();

        return memory;
    }

    static inline void* reallocate(void* memory, size_type size) noexcept {
        memory = 
#    if __has_builtin(__builtin_realloc)
        __builtin_realloc(memory, size);
#    else
        ::std::realloc(memory, size);
#    endif
        if (!memory) [[unlikely]]
            terminate();

        return memory;
    }

    static inline void deallocate(void* memory, size_type, size_type) noexcept {
        if (!memory)
            return;

#    if __has_builtin(__builtin_free)
        __builtin_free(memory);
#    else
        ::std::free(memory);
#    endif
    }

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return size_type(-1) / sizeof(::std::byte);
    }
};
// clang-format on

} // namespace flux::fou::detail