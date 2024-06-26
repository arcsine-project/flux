#include <flux/foundation.hpp>
#include <flux/foundation/memory/detail/test_allocator.hpp>

#include <catch2/catch.hpp>

struct [[nodiscard]] typeless_allocator final {
    using size_type       = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;

    typeless_allocator() = default;

    [[nodiscard]] void* allocate(size_type /* bytes */) {
        return nullptr;
    }

    void deallocate(void*, size_type) noexcept {}
};

using namespace flux;

// clang-format off
using std_stateful_allocator = fou::std_allocator_adapter<int, fou::test_allocator<::std::unordered_map>>;
static_assert(meta::same_as<std_stateful_allocator::propagate_on_container_copy_assignment, meta::false_type>);
static_assert(meta::same_as<std_stateful_allocator::propagate_on_container_move_assignment, meta::false_type>);
static_assert(meta::same_as<std_stateful_allocator::propagate_on_container_swap, meta::false_type>);

using std_stateless_allocator = fou::std_allocator_adapter<int, ::std::allocator<int>>;
static_assert(meta::same_as<std_stateless_allocator::propagate_on_container_copy_assignment, meta::true_type>);
static_assert(meta::same_as<std_stateless_allocator::propagate_on_container_move_assignment, meta::true_type>);
static_assert(meta::same_as<std_stateless_allocator::propagate_on_container_swap, meta::false_type>);

using std_any_allocator = fou::std_any_allocator<int>;
static_assert(meta::same_as<std_any_allocator::propagate_on_container_copy_assignment, meta::false_type>);
static_assert(meta::same_as<std_any_allocator::propagate_on_container_move_assignment, meta::false_type>);
static_assert(meta::same_as<std_any_allocator::propagate_on_container_swap, meta::false_type>);
// clang-format on

TEST_CASE("fou::std_allocator_adapter", "[flux-memory/std_allocator.hpp]") {
    using test_allocator = fou::test_allocator<::std::unordered_map>;

    SECTION("test std_stateful_allocator") {
        test_allocator         test;
        std_stateful_allocator allocator{test};

        auto* ptr = allocator.allocate(sizeof(int));
        CHECK(fou::is_aligned(ptr, alignof(int)));
        CHECK(test.allocated_count() == 1u);
        CHECK(test.deallocated_count() == 0u);

        allocator.deallocate(ptr, sizeof(int));
        CHECK(test.allocated_count() == 0u);
        CHECK(test.deallocated_count() == 1u);

        (void)allocator.select_on_container_copy_construction();
    }

    SECTION("test std_stateless_allocator") {
        std_stateless_allocator allocator;

        auto* ptr = allocator.allocate(1);
        CHECK(fou::is_aligned(ptr, alignof(int)));
        allocator.deallocate(ptr, 1);
    }

    SECTION("test std_any_allocator") {
        ::std::allocator<int> const int_allocator{};
        std_any_allocator           allocator{int_allocator};

        auto* ptr = allocator.allocate(sizeof(int));
        CHECK(fou::is_aligned(ptr, alignof(int)));
        allocator.deallocate(ptr, sizeof(int));
    }

    SECTION("test std_any_allocator with typeless_allocator") {
        std_any_allocator allocator{typeless_allocator{}};

        auto* ptr = allocator.allocate(1);
        CHECK_FALSE(ptr);
        allocator.deallocate(ptr, 1);
    }

    SECTION("test compare") {
        test_allocator test;

        fou::std_allocator_adapter<int, typeless_allocator>  a0{typeless_allocator{}};
        fou::std_allocator_adapter<char, typeless_allocator> a1{typeless_allocator{}};
        CHECK(a0 == a1);

        fou::std_allocator_adapter<int, test_allocator> const  a2{test};
        fou::std_allocator_adapter<char, test_allocator> const a3{test};
        CHECK(a2 == a3);
    }
}