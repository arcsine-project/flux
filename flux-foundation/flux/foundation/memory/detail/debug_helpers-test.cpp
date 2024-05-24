#include <flux/foundation.hpp>

#include <catch2/catch.hpp>

using namespace flux::fou;
using namespace flux::fou::detail;

TEST_CASE("fou::detail::debug_fill", "[flux-memory/debug_helpers.hpp]") {
    debug_magic magic_values[10];
    for (auto& magic_value : magic_values) {
        magic_value = debug_magic::freed_memory;
    }

    debug_fill(magic_values, sizeof(magic_values), debug_magic::new_memory);
#if FLUX_MEMORY_DEBUG_FILL
    for (auto magic_value : magic_values) {
        CHECK(magic_value == debug_magic::new_memory);
    }
#else
    for (auto magic_value : magic_values) {
        CHECK(magic_value == debug_magic::freed_memory);
    }
#endif
}

TEST_CASE("fou::detail::debug_is_filled", "[flux-memory/debug_helpers.hpp]") {
    debug_magic magic_values[10];
    for (auto& magic_value : magic_values) {
        magic_value = debug_magic::freed_memory;
    }

    CHECK(debug_is_filled(magic_values, sizeof(magic_values), debug_magic::freed_memory) ==
          nullptr);

    magic_values[5] = debug_magic::new_memory;
    auto ptr        = static_cast<debug_magic*>(
            debug_is_filled(magic_values, sizeof(magic_values), debug_magic::freed_memory));
#if FLUX_MEMORY_DEBUG_FILL
    CHECK(ptr == magic_values + 5);
#else
    CHECK(ptr == nullptr);
#endif
}

TEST_CASE("fou::detail::debug_fill_new/free", "[flux-memory/debug_helpers.hpp]") {
    debug_magic magic_values[10];

    auto result = debug_fill_new(magic_values, 8 * sizeof(debug_magic), sizeof(debug_magic));
    auto offset = static_cast<debug_magic*>(result) - magic_values;
    auto expected_offset = static_cast<::std::int32_t>(debug_fence_size ? sizeof(debug_magic) : 0);
    CHECK(offset == expected_offset);

#if FLUX_MEMORY_DEBUG_FILL
#    if FLUX_MEMORY_DEBUG_FENCE
    CHECK(magic_values[0] == debug_magic::fence_memory);
    CHECK(magic_values[9] == debug_magic::fence_memory);
    const auto start = 1;
#    else
    const auto start = 0;
#    endif
    for (auto i = start; i < start + 8; ++i) {
        CHECK(magic_values[i] == debug_magic::new_memory);
    }
#endif

    result = debug_fill_free(result, 8 * sizeof(debug_magic), sizeof(debug_magic));
    CHECK(static_cast<debug_magic*>(result) == magic_values);

#if FLUX_MEMORY_DEBUG_FILL
#    if FLUX_MEMORY_DEBUG_FENCE
    CHECK(magic_values[0] == debug_magic::fence_memory);
    CHECK(magic_values[9] == debug_magic::fence_memory);
#    endif
    for (auto i = start; i < start + 8; ++i) {
        CHECK(magic_values[i] == debug_magic::freed_memory);
    }
#endif
}