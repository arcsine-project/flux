#include <flux/foundation.hpp>
#include <flux/foundation/memory/detail/test_allocator.hpp>

#include <catch2/catch.hpp>

TEST_CASE("fou::memory_stack", "[flux-memory/memory_stack.hpp]") {
    using namespace flux::fou;
    using test_allocator = test_allocator<::std::unordered_map>;
    using memory_stack   = memory_stack<allocator_reference<test_allocator>>;

    detail::memory_stack_leak_handler()(0);

    test_allocator allocator;
    memory_stack   stack{memory_stack::min_block_size(100), allocator};
    CHECK(stack.allocator().block_size() == 232u);

    CHECK(allocator.allocated_count() == 1u);
    CHECK(stack.capacity() == 100u);

    auto capacity = stack.capacity();
    SECTION("empty unwind") {
        auto marker = stack.top();
        stack.unwind(marker);
        CHECK(capacity <= 100u);
        CHECK(allocator.allocated_count() == 1u);
        CHECK(allocator.deallocated_count() == 0u);

        auto m0  = marker;
        auto m1  = marker;
        m0.index = 0;
        m1.index = 1;
        CHECK_FALSE(m0 == m1);
    }

    SECTION("normal allocation/unwind") {
        stack.allocate(10u, 1u);
        CHECK(stack.capacity() == capacity - 10 - 2 * detail::debug_fence_size);

        auto marker = stack.top();
        auto memory = composable_traits<memory_stack>::try_allocate_node(stack, 10u, 16u);
        CHECK(is_aligned(memory, 16u));

        stack.unwind(marker);
        CHECK(stack.capacity() == capacity - 10 - 2 * detail::debug_fence_size);

        auto array = composable_traits<memory_stack>::try_allocate_array(stack, 1u, 10u, 16u);
        CHECK(array == memory);
        CHECK(allocator.allocated_count() == 1u);
        CHECK(allocator.deallocated_count() == 0u);
        CHECK(composable_traits<memory_stack>::try_deallocate_array(stack, array, 1u, 10u, 16u));
    }

    SECTION("multiple block allocation/unwind") {
        // note: tests are mostly hoping not to get a segfault

        stack.allocate(10, 1);
        auto marker = stack.top();

        auto old_next = allocator_traits<memory_stack>::max_node_size(stack);

        stack.allocate(100, 1);
        CHECK(allocator_traits<memory_stack>::max_array_size(stack) > old_next);
        CHECK(allocator_traits<memory_stack>::max_alignment(stack) > 0);
        CHECK(allocator.allocated_count() == 2u);
        CHECK(allocator.deallocated_count() == 0u);

        auto m2 = stack.top();
        CHECK(marker < m2);
        stack.allocate(10, 1);
        stack.unwind(m2);
        stack.allocate(20, 1);

        stack.unwind(marker);
        CHECK(allocator.allocated_count() == 2u);
        CHECK(allocator.deallocated_count() == 0u);

        stack.allocate(10, 1);

        stack.shrink_to_fit();
        CHECK(allocator.allocated_count() == 1u);
        CHECK(allocator.deallocated_count() == 1u);
    }

    SECTION("move") {
        auto other  = ::std::move(stack);
        auto marker = other.top();
        other.allocate(10, 1);
        CHECK(allocator.allocated_count() == 1u);

        stack = ::std::move(other);
        CHECK(allocator.allocated_count() == 1u);
        stack.unwind(marker);
    }

    SECTION("marker comparision") {
        auto m1 = stack.top();
        auto m2 = stack.top();
        CHECK(m1 == m2);

        stack.allocate(1, 1);
        auto m3 = stack.top();
        CHECK(m1 < m3);

        stack.unwind(m2);
        CHECK(stack.top() == m2);
    }

    SECTION("unwinder") {
        auto marker = stack.top();
        {
            memory_stack_unwinder<decltype(stack)> unwinder(stack);
            stack.allocate(10, 1);
            CHECK(unwinder.will_unwind());
            CHECK(&unwinder.stack() == &stack);
            CHECK(unwinder.marker() == marker);
        }
        CHECK(stack.top() == marker);

        memory_stack_unwinder<decltype(stack)> unwinder(stack);
        stack.allocate(10, 1);
        unwinder.unwind();
        CHECK(stack.top() == marker);
        CHECK(unwinder.will_unwind());

        {
            memory_stack_unwinder<decltype(stack)> unwinder2(stack);
            stack.allocate(10, 1);
            unwinder2.release();
            CHECK(!unwinder2.will_unwind());
        }
        CHECK(stack.top() > marker);
        marker = stack.top();

        unwinder.release(); // need to release
        unwinder = memory_stack_unwinder<decltype(stack)>(stack);
        CHECK(unwinder.will_unwind());
        CHECK(unwinder.marker() == marker);
        CHECK(&unwinder.stack() == &stack);
        auto unwinder2 = ::std::move(unwinder);
        CHECK(unwinder2.will_unwind());
        CHECK(&unwinder2.stack() == &stack);
        CHECK(unwinder2.marker() == marker);
        CHECK(!unwinder.will_unwind());
    }

    SECTION("unwinder move") {
        auto marker = stack.top();

        memory_stack_unwinder<decltype(stack)> unwinder1(stack);
        memory_stack_unwinder<decltype(stack)> unwinder2(stack);

        stack.allocate(10, 1);

        CHECK(unwinder1.will_unwind());
        CHECK(&unwinder1.stack() == &stack);
        CHECK(unwinder1.marker() == marker);

        CHECK(unwinder2.will_unwind());
        CHECK(&unwinder2.stack() == &stack);
        CHECK(unwinder2.marker() == marker);

        unwinder2 = ::std::move(unwinder1);
        CHECK(stack.top() == marker);
        CHECK_FALSE(unwinder1.will_unwind());
        unwinder2.release();
    }

    SECTION("overaligned") {
        auto align = 2 * detail::max_alignment;
        auto mem   = stack.allocate(align, align);
        CHECK(is_aligned(mem, align));
    }
}