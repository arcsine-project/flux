#include <flux/foundation.hpp>

#include <catch2/catch.hpp>

static_assert(flux::meta::trivially_relocatable<int>);
static_assert(flux::meta::trivially_relocatable<const int>);
static_assert(flux::meta::trivially_relocatable<int*>);
static_assert(flux::meta::trivially_relocatable<int (*)()>);

static_assert(flux::meta::trivially_relocatable<int[]>);
static_assert(flux::meta::trivially_relocatable<int const[]>);
static_assert(flux::meta::trivially_relocatable<int[4]>);
static_assert(flux::meta::trivially_relocatable<int const[4]>);

static_assert(not flux::meta::trivially_relocatable<void>);
static_assert(not flux::meta::trivially_relocatable<void const>);
static_assert(not flux::meta::trivially_relocatable<int()>);

static_assert(flux::meta::relocatable<int>);
static_assert(flux::meta::relocatable<const int>);
static_assert(flux::meta::relocatable<int*>);
static_assert(flux::meta::relocatable<int (*)()>);

static_assert(not flux::meta::relocatable<int[]>);
static_assert(not flux::meta::relocatable<int const[]>);
static_assert(not flux::meta::relocatable<int[4]>);
static_assert(not flux::meta::relocatable<int const[4]>);

static_assert(not flux::meta::relocatable<void>);
static_assert(not flux::meta::relocatable<void const>);
static_assert(not flux::meta::relocatable<int()>);

struct counted {
    int value;

    explicit counted(int&& v) noexcept : value{v} {
        v = 0;
        ++count;
        ++constructed;
    }
    counted(counted const&) noexcept {
        assert(false);
    }
    ~counted() {
        assert(count > 0);
        --count;
    }
    // clang-format off
    static int  count;
    static int  constructed;
    static void reset() noexcept { count = constructed = 0; }
    // clang-format on

    friend void operator&(counted) = delete;
};
int counted::count       = 0;
int counted::constructed = 0;

TEST_CASE("flux::fou::uninitialized_value_construct",
          "[flux-foundation/uninitialized_algorithms.hpp]") {
    int  value;
    auto new_value = flux::fou::uninitialized_value_construct(&value);
    CHECK(value == *new_value);
    CHECK(value == 0);
}

TEST_CASE("flux::fou::uninitialized_value_construct_n",
          "[flux-foundation/uninitialized_algorithms.hpp]") {
    int values[5];
    flux::fou::uninitialized_value_construct_n(values, 5);
    for (auto value : values) {
        CHECK(value == 0);
    }
}

TEST_CASE("flux::fou::uninitialized_relocate", "[flux-foundation/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    using InputIterator  = int*;
    using OutputIterator = counted*;
    auto const N         = 5;
    int        values[N] = {1, 2, 3, 4, 5};
    // clang-format off
        alignas(counted)
        std::byte pool[sizeof(counted) * N] = {};
        counted*  counted_pointer           = reinterpret_cast<counted*>(pool);
    // clang-format on
    auto result = uninitialized_relocate(InputIterator(values), InputIterator(values + 1),
                                         OutputIterator(counted_pointer));
    CHECK(result == OutputIterator(counted_pointer + 1));
    CHECK(counted::constructed == 1);
    CHECK(counted::count == 1);
    CHECK(counted_pointer[0].value == 1);
    CHECK(values[0] == 0);

    result = uninitialized_relocate(InputIterator(values + 1), InputIterator(values + N),
                                    OutputIterator(counted_pointer + 1));
    CHECK(result == OutputIterator(counted_pointer + N));
    CHECK(counted::count == 5);
    CHECK(counted::constructed == 5);
    CHECK(counted_pointer[1].value == 2);
    CHECK(counted_pointer[2].value == 3);
    CHECK(counted_pointer[3].value == 4);
    CHECK(counted_pointer[4].value == 5);
    CHECK(values[1] == 0);
    CHECK(values[2] == 0);
    CHECK(values[3] == 0);
    CHECK(values[4] == 0);

    destroy_range(counted_pointer, counted_pointer + N);
    CHECK(counted::count == 0);
}