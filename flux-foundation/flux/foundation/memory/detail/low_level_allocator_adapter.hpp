#pragma once
#include <flux/foundation/memory/align.hpp>
#include <flux/foundation/memory/detail/debug_helpers.hpp>

namespace flux::fou::detail {

// clang-format off
template <typename Allocator>
struct [[nodiscard]] low_level_allocator_leak_handler {
    constexpr void operator()(::std::ptrdiff_t amount) noexcept {
        debug_handle_memory_leak(Allocator::info(), amount);
    }
};

template <typename Allocator>
concept low_level_allocator =
    requires(::std::size_t size, ::std::size_t align) {
        // A low-level allocator should have static allocate/deallocate functions...
        { Allocator::allocate  (         size, align) } noexcept -> meta::same_as<void*>;
        { Allocator::deallocate(nullptr, size, align) } noexcept -> meta::same_as<void>;
        // ...as well as auxiliary utilities.
        { Allocator::max_size() } noexcept -> meta::same_as<::std::size_t>;
        { Allocator::info()     } noexcept -> meta::same_as<allocator_info>;
    };
// clang-format on

template <low_level_allocator Allocator>
struct [[nodiscard]] low_level_allocator_adapter
        : global_leak_detector<low_level_allocator_leak_handler<Allocator>> {
    using allocator_type    = Allocator;
    using allocation_result = typename allocator_type::allocation_result;
    using size_type         = typename allocator_type::size_type;
    using difference_type   = typename allocator_type::difference_type;
    using leak_detector     = global_leak_detector<low_level_allocator_leak_handler<Allocator>>;
    using stateful          = meta::false_type;

    constexpr low_level_allocator_adapter() noexcept = default;
    constexpr ~low_level_allocator_adapter()         = default;

    // clang-format off
    constexpr low_level_allocator_adapter(low_level_allocator_adapter&&) noexcept = default;
    constexpr low_level_allocator_adapter& operator=(low_level_allocator_adapter&&) noexcept = default;
    // clang-format on

    constexpr void* allocate(size_type size) noexcept {
        auto actual_size = FLUX_MEMORY_DEBUG_FENCE_SIZE(size);
        auto memory      = allocator_type::allocate(actual_size);

        leak_detector::on_allocate(actual_size);

        return debug_fill_new(memory, size, max_alignment);
    }

    constexpr auto allocate_at_least(size_type size) noexcept {
        auto actual_size = FLUX_MEMORY_DEBUG_FENCE_SIZE(size);
        auto [memory, n] = allocator_type::allocate_at_least(actual_size);

        leak_detector::on_allocate(n);

        FLUX_MEMORY_DEBUG_UNFENCE_SIZE(n);
        return allocation_result{debug_fill_new(memory, size, max_alignment), n};
    }

    constexpr void deallocate(void* node, size_type size) noexcept {
        auto actual_size = FLUX_MEMORY_DEBUG_FENCE_SIZE(size);
        auto memory      = debug_fill_free(node, size, max_alignment);

        allocator_type::deallocate(memory);

        leak_detector::on_deallocate(actual_size);
    }

    constexpr void* allocate_node(size_type size, size_type alignment) noexcept {
        auto actual_size = FLUX_MEMORY_DEBUG_FENCE_SIZE(size);
        auto memory      = allocator_type::allocate(actual_size, alignment);

        leak_detector::on_allocate(actual_size);

        return debug_fill_new(memory, size, max_alignment);
    }

    constexpr auto allocate_node_at_least(size_type size, size_type /* alignment */) noexcept {
        auto actual_size = FLUX_MEMORY_DEBUG_FENCE_SIZE(size);
        auto [memory, n] = allocator_type::allocate_at_least(actual_size);

        leak_detector::on_allocate(actual_size);

        return allocation_result{debug_fill_new(memory, n, max_alignment), n};
    }

    constexpr void deallocate_node(void* node, size_type size, size_type alignment) noexcept {
        auto actual_size = FLUX_MEMORY_DEBUG_FENCE_SIZE(size);
        auto memory      = debug_fill_free(node, size, max_alignment);

        allocator_type::deallocate(memory, actual_size, alignment);

        leak_detector::on_deallocate(actual_size);
    }

    constexpr size_type max_node_size() const noexcept {
        return allocator_type::max_size();
    }
};

#define FLUX_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(allocator, name)                                     \
    FLUX_MEMORY_GLOBAL_LEAK_DETECTOR(low_level_allocator_leak_handler<allocator>, name)

} // namespace flux::fou::detail