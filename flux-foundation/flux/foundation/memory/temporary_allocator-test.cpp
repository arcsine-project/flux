#include <flux/foundation.hpp>

#include <catch2/catch.hpp>

using namespace flux;

template <typename T>
using stack_vector = ::std::vector<T, fou::std_allocator_adapter<T, fou::temporary_allocator>>;

TEST_CASE("fou::temporary_allocator", "[flux-memory/temporary_allocator.hpp]") {

    SECTION("test temporary allocator") {
        fou::temporary_allocator allocator;

        auto ptr = allocator.allocate(sizeof(char), alignof(char));
        INFO("ptr = " << ptr << ", max_alignment = " << fou::detail::max_alignment);
        CHECK(fou::is_aligned(ptr, alignof(char)));
        CHECK_FALSE(fou::is_aligned(ptr, fou::detail::max_alignment));
    }

    SECTION("test temporary vector") {
        fou::temporary_allocator allocator;

        stack_vector<int> v{allocator};
        v.push_back(2023);
        CHECK(2023 == v[0]);
    }
}