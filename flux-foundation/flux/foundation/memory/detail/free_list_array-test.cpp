#include <flux/foundation.hpp>

#include <catch2/catch.hpp>

using namespace flux::fou;

using log2_policy = detail::log2_access_policy;

TEST_CASE("fou::detail::log2_access_policy", "[flux-memory/free_list_array.hpp]") {
    REQUIRE(log2_policy::index_from_size(1) == 0u);
    REQUIRE(log2_policy::index_from_size(2) == 1u);
    REQUIRE(log2_policy::index_from_size(3) == 2u);
    REQUIRE(log2_policy::index_from_size(4) == 2u);
    REQUIRE(log2_policy::index_from_size(5) == 3u);
    REQUIRE(log2_policy::index_from_size(6) == 3u);
    REQUIRE(log2_policy::index_from_size(8) == 3u);
    REQUIRE(log2_policy::index_from_size(9) == 4u);

    REQUIRE(log2_policy::size_from_index(0) == 1u);
    REQUIRE(log2_policy::size_from_index(1) == 2u);
    REQUIRE(log2_policy::size_from_index(2) == 4u);
    REQUIRE(log2_policy::size_from_index(3) == 8u);
}

TEST_CASE("fou::detail::free_list_array", "[flux-memory/free_list_array.hpp]") {
    using fixed_stack = detail::fixed_stack;
    using free_list   = detail::unordered_free_list;
    static_allocator_storage<1024> memory;
    fixed_stack                    stack(&memory);

    SECTION("non power of two max size, normal list") {
        using log2_free_list_array = detail::free_list_array<free_list, log2_policy>;
        log2_free_list_array array(stack, stack.top() + 1024, 15);
        CHECK(array.max_node_size() == 16u);
        CHECK(array.size() <= 5u);

        CHECK(array[1].node_size() == free_list::min_node_size);
        CHECK(array[2].node_size() == free_list::min_node_size);
        CHECK(array[9].node_size() == 16u);
        CHECK(array[15].node_size() == 16u);
    }
}