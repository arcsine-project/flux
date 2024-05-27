#pragma once
#include <flux/foundation/memory/debugging.hpp>
#include <flux/foundation/utility/terminate.hpp>

#include <flux/platform/win32/api.hpp>

namespace flux::fou::detail {

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
[[__gnu__::__returns_nonnull__]]
#endif
inline void* win32_heapalloc_common(::std::size_t size, ::std::uint_least32_t flag) noexcept {
    if (0 == size) {
        size = 1u;
    }

    auto* memory = win32::HeapAlloc(win32::GetProcessHeap(), flag, size);
    if (!memory) [[unlikely]] {
        fast_terminate();
    }

    return memory;
}

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
[[__gnu__::__returns_nonnull__]]
#endif
inline void* win32_heaprealloc_common(void* address, ::std::size_t size,
                                      ::std::uint_least32_t flag) noexcept {
    if (0 == size) {
        size = 1u;
    }
    if (!address) [[unlikely]] {
        return win32::HeapAlloc(win32::GetProcessHeap(), flag, size);
    }

    auto* memory = win32::HeapReAlloc(win32::GetProcessHeap(), flag, address, size);
    if (!memory) [[unlikely]] {
        fast_terminate();
    }

    return memory;
}

// clang-format off
// Low-level allocator.
struct [[nodiscard]] win32_heap_allocator final {
    using size_type       = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;

    static inline auto info() noexcept {
        return allocator_info{"flux::fou::detail::win32_heap_allocator", nullptr};
    }

#if __has_cpp_attribute(__gnu__::__malloc__)
    [[__gnu__::__malloc__]]
#endif
    static inline void* allocate(size_type size, size_type) noexcept {
        return win32_heapalloc_common(size, 0u);
    }

    static inline void* reallocate(void* memory, size_type size) noexcept {
        return win32_heaprealloc_common(memory, size, 0u);
    }

    static inline void deallocate(void* memory, size_type, size_type) noexcept {
        if (!memory)
            return;

        win32::HeapFree(win32::GetProcessHeap(), 0u, memory);
    }

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return 0xFFFFFFFFFFFFFFE0;
    }
};
// clang-format on

} // namespace flux::fou::detail