#include <flux/foundation.hpp>

#include <array> // TODO: Replace with own implementation.
#include <catch2/catch.hpp>

using namespace flux;

// clang-format off
template <typename T>
struct value_wrapper {
    T value;

    constexpr bool operator==(value_wrapper const&) const noexcept = default;
};

namespace type_tag {
struct move_constructible_and_destructible {};
struct     trivially_default_constructible {};
struct non_trivially_default_constructible {};
struct non_trivially_copyable              {};
struct non_trivially_movable               {};
} // namespace type_tag

template <typename TagType, typename ValueType>
struct test_type;
// clang-format on

template <typename T>
struct test_type<type_tag::trivially_default_constructible, T> final : value_wrapper<T> {};
using trivially_default_constructible_t = test_type<type_tag::trivially_default_constructible, int>;
static_assert(meta::trivially_default_constructible<trivially_default_constructible_t>);
using trivially_copyable_t = trivially_default_constructible_t;
static_assert(meta::trivially_copyable<trivially_default_constructible_t>);
static_assert(meta::trivially_relocatable<trivially_default_constructible_t>);

template <typename T>
struct test_type<type_tag::non_trivially_default_constructible, T> final : value_wrapper<T> {
    constexpr test_type() noexcept : value_wrapper<T>() {}
};
using non_trivially_default_constructible_t =
        test_type<type_tag::non_trivially_default_constructible, int>;
static_assert(not meta::trivially_default_constructible<non_trivially_default_constructible_t>);

template <typename T>
struct test_type<type_tag::non_trivially_copyable, T> final : value_wrapper<T> {
    constexpr test_type() noexcept : value_wrapper<T>() {}
    constexpr test_type(T value) noexcept : value_wrapper<T>(value) {}
    constexpr test_type(test_type const& other) noexcept : value_wrapper<T>(other) {}
};
using non_trivially_copyable_t = test_type<type_tag::non_trivially_copyable, int>;
static_assert(not meta::trivially_copyable<non_trivially_copyable_t>);

template <typename T>
struct test_type<type_tag::non_trivially_movable, T> final : value_wrapper<T> {
    constexpr test_type() noexcept : value_wrapper<T>() {}
    constexpr test_type(T value) noexcept : value_wrapper<T>(value) {}
    constexpr test_type(test_type&& other) noexcept : value_wrapper<T>(::std::move(other)) {}
};
using non_trivially_movable_t = test_type<type_tag::non_trivially_movable, int>;
static_assert(not meta::trivially_copyable<non_trivially_movable_t>);
static_assert(not meta::trivially_relocatable<non_trivially_movable_t>);
static_assert(meta::relocatable<non_trivially_movable_t>);

struct non_trivially_movable_counter_t final {
    int value;

    explicit non_trivially_movable_counter_t(int&& v) noexcept : value{v} {
        v = 0;
        ++count;
        ++constructed;
    }

    non_trivially_movable_counter_t(non_trivially_movable_counter_t const&) noexcept {
        assert(false);
    }

    ~non_trivially_movable_counter_t() {
        assert(count > 0);
        --count;
    }

    // clang-format off
    static int  count;
    static int  constructed;
    static void reset() noexcept { count = constructed = 0; }
    // clang-format on

    friend void operator&(non_trivially_movable_counter_t) = delete;
};
int non_trivially_movable_counter_t::count       = 0;
int non_trivially_movable_counter_t::constructed = 0;
static_assert(not meta::trivially_copyable<non_trivially_movable_counter_t>);
using relocatable_counter_t = non_trivially_movable_counter_t;

TEST_CASE("fou::ranges::uninitialized_default_construct", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially default constructible at compile-time") {
        using trivially_default_constructible_array_t =
                ::std::array<trivially_default_constructible_t, 3>;
        constexpr auto result_first_last = [] noexcept {
            trivially_default_constructible_array_t array;
            ranges::uninitialized_default_construct(array.begin(), array.end());
            return array;
        }();
        STATIC_REQUIRE(result_first_last == trivially_default_constructible_array_t{});

        constexpr auto result_range = [] noexcept {
            trivially_default_constructible_array_t array;
            ranges::uninitialized_default_construct(array);
            return array;
        }();
        STATIC_REQUIRE(result_range == trivially_default_constructible_array_t{});
    }

    SECTION("trivially default constructible at run-time (skip initialization)") {
        using trivially_default_constructible_array_t =
                ::std::array<trivially_default_constructible_t, 5>;
        {
            trivially_default_constructible_array_t array;
            auto it = ranges::uninitialized_default_construct(array.begin(), array.end());
            CHECK(array.end() == it); // assume that there are garbage values
        }
        {
            trivially_default_constructible_array_t array;
            auto it = ranges::uninitialized_default_construct(array);
            CHECK(array.end() == it); // assume that there are garbage values
        }
    }

    SECTION("non trivially default constructible at compile-time") {
        using non_trivially_default_constructible_array_t =
                ::std::array<non_trivially_default_constructible_t, 3>;
        constexpr auto result_first_last = [] noexcept {
            non_trivially_default_constructible_array_t array;
            ranges::uninitialized_default_construct(array.begin(), array.end());
            return array;
        }();
        STATIC_REQUIRE(result_first_last == non_trivially_default_constructible_array_t{});

        constexpr auto result_range = [] noexcept {
            non_trivially_default_constructible_array_t array;
            ranges::uninitialized_default_construct(array);
            return array;
        }();
        STATIC_REQUIRE(result_range == non_trivially_default_constructible_array_t{});
    }

    SECTION("non trivially default constructible at run-time") {
        using non_trivially_default_constructible_array_t =
                ::std::array<non_trivially_default_constructible_t, 5>;
        {
            non_trivially_default_constructible_array_t array;
            ranges::uninitialized_default_construct(array.begin(), array.end());
            CHECK(array == non_trivially_default_constructible_array_t{});
        }
        {
            non_trivially_default_constructible_array_t array;
            ranges::uninitialized_default_construct(array);
            CHECK(array == non_trivially_default_constructible_array_t{});
        }
    }
}

TEST_CASE("fou::ranges::uninitialized_default_construct_n",
          "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially default constructible at compile-time") {
        using trivially_default_constructible_array_t =
                ::std::array<trivially_default_constructible_t, 3>;
        constexpr auto result = [] noexcept {
            trivially_default_constructible_array_t array;
            ranges::uninitialized_default_construct_n(array.begin(), array.size());
            return array;
        }();
        STATIC_REQUIRE(result == trivially_default_constructible_array_t{});
    }

    SECTION("trivially default constructible at run-time (skip initialization)") {
        using trivially_default_constructible_array_t =
                ::std::array<trivially_default_constructible_t, 5>;
        trivially_default_constructible_array_t array;
        auto it = ranges::uninitialized_default_construct_n(array.begin(), array.size());
        CHECK(array.end() == it); // assume that there are garbage values
    }

    SECTION("non trivially default constructible at compile-time") {
        using non_trivially_default_constructible_array_t =
                ::std::array<non_trivially_default_constructible_t, 3>;
        constexpr auto result = [] noexcept {
            non_trivially_default_constructible_array_t array;
            ranges::uninitialized_default_construct_n(array.begin(), array.size());
            return array;
        }();
        STATIC_REQUIRE(result == non_trivially_default_constructible_array_t{});
    }

    SECTION("non trivially default constructible at run-time") {
        using non_trivially_default_constructible_array_t =
                ::std::array<non_trivially_default_constructible_t, 5>;
        non_trivially_default_constructible_array_t array;
        ranges::uninitialized_default_construct_n(array.begin(), array.size());
        CHECK(array == non_trivially_default_constructible_array_t{});
    }
}

TEST_CASE("fou::ranges::uninitialized_value_construct", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially copyable at compile-time") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        constexpr auto result_first_last = [] noexcept {
            trivially_copyable_array_t array;
            ranges::uninitialized_value_construct(array.begin(), array.end());
            return array;
        }();
        STATIC_REQUIRE(result_first_last == trivially_copyable_array_t{});

        constexpr auto result_range = [] noexcept {
            trivially_copyable_array_t array;
            ranges::uninitialized_value_construct(array);
            return array;
        }();
        STATIC_REQUIRE(result_range == trivially_copyable_array_t{});
    }

    SECTION("trivially copyable at run-time (memset initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 5>;
        static_assert(meta::use_memset_value_construct<trivially_copyable_array_t::iterator>);
        {
            trivially_copyable_array_t array;

            auto it = ranges::uninitialized_value_construct(array.begin(), array.end());
            CHECK(array.end() == it);
            CHECK(array == trivially_copyable_array_t{});
        }
        {
            trivially_copyable_array_t array;

            auto it = ranges::uninitialized_value_construct(array);
            CHECK(array.end() == it);
            CHECK(array == trivially_copyable_array_t{});
        }
    }

    SECTION("non trivially copyable at compile-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 3>;
        constexpr auto result_first_last     = [] noexcept {
            non_trivially_copyable_array_t array;
            ranges::uninitialized_value_construct(array.begin(), array.end());
            return array;
        }();
        STATIC_REQUIRE(result_first_last == non_trivially_copyable_array_t{});

        constexpr auto result_range = [] noexcept {
            non_trivially_copyable_array_t array;
            ranges::uninitialized_value_construct(array);
            return array;
        }();
        STATIC_REQUIRE(result_range == non_trivially_copyable_array_t{});
    }

    SECTION("non trivially copyable at run-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 5>;
        static_assert(
                not meta::use_memset_value_construct<non_trivially_copyable_array_t::iterator>);
        {
            non_trivially_copyable_array_t array;

            auto it = ranges::uninitialized_value_construct(array.begin(), array.end());
            CHECK(array.end() == it);
            CHECK(array == non_trivially_copyable_array_t{});
        }
        {
            non_trivially_copyable_array_t array;

            auto it = ranges::uninitialized_value_construct(array);
            CHECK(array.end() == it);
            CHECK(array == non_trivially_copyable_array_t{});
        }
    }
}

TEST_CASE("fou::ranges::uninitialized_value_construct_n", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially copyable at compile-time") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        constexpr auto result_first_last = [] noexcept {
            trivially_copyable_array_t array;
            ranges::uninitialized_value_construct_n(array.begin(), array.size());
            return array;
        }();
        STATIC_REQUIRE(result_first_last == trivially_copyable_array_t{});
    }

    SECTION("trivially copyable at run-time (memset initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 5>;
        static_assert(meta::use_memset_value_construct<trivially_copyable_array_t::iterator>);

        trivially_copyable_array_t array;
        auto it = ranges::uninitialized_value_construct_n(array.begin(), array.size());
        CHECK(array.end() == it);
        CHECK(array == trivially_copyable_array_t{});
    }

    SECTION("non trivially copyable at compile-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 3>;
        constexpr auto result_first_last     = [] noexcept {
            non_trivially_copyable_array_t array;
            ranges::uninitialized_value_construct_n(array.begin(), array.size());
            return array;
        }();
        STATIC_REQUIRE(result_first_last == non_trivially_copyable_array_t{});
    }

    SECTION("non trivially copyable at run-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 5>;
        non_trivially_copyable_array_t array;

        auto it = ranges::uninitialized_value_construct_n(array.begin(), array.size());
        CHECK(array.end() == it);
        CHECK(array == non_trivially_copyable_array_t{});
    }
}

TEST_CASE("fou::ranges::uninitialized_copy", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially copyable at compile-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        constexpr auto result_first_last = [] noexcept {
            trivially_copyable_array_t array1 = {{{1}, {2}, {3}}};
            trivially_copyable_array_t array2;
            [[maybe_unused]] auto out = ranges::uninitialized_copy(array1.begin(), array1.end(),
                                                                   array2.begin(), array2.end());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last == trivially_copyable_array_t{{{1}, {2}, {3}}});

        constexpr auto result_first_last_result = [] noexcept {
            trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
            trivially_copyable_array_t array2;
            ranges::uninitialized_copy(array1.begin(), array1.end(), array2.begin());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last_result == trivially_copyable_array_t{{{3}, {2}, {1}}});

        constexpr auto result_range = [] noexcept {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};
            [[maybe_unused]] auto      out    = ranges::uninitialized_copy(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == trivially_copyable_array_t{{{4}, {5}, {6}}});
    }

    SECTION("trivially copyable at run-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        {
            trivially_copyable_array_t array1 = {{{1}, {2}, {3}}};
            trivially_copyable_array_t array2;

            auto [it1, it2] = ranges::uninitialized_copy(array1.begin(), array1.end(),
                                                         array2.begin(), array2.end());
            CHECK(array1.end() == it1);
            CHECK(array2.end() == it2);
            CHECK(array2 == trivially_copyable_array_t{{{1}, {2}, {3}}});
        }
        {
            trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
            trivially_copyable_array_t array2;

            auto it = ranges::uninitialized_copy(array1.begin(), array1.end(), array2.begin());
            CHECK(array2.end() == it);
            CHECK(array2 == trivially_copyable_array_t{{{3}, {2}, {1}}});
        }
        {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};

            auto [it1, it2] = ranges::uninitialized_copy(array2, array1);
            CHECK(array1.end() == it2);
            CHECK(array2.end() == it1);
            CHECK(array1 == trivially_copyable_array_t{{{4}, {5}, {6}}});
        }
    }

    SECTION("non trivially copyable at compile-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_copyable_array_t::iterator,
                                           non_trivially_copyable_array_t::iterator>);
        constexpr auto result_first_last = [] noexcept {
            non_trivially_copyable_array_t array1 = {1, 2, 3};
            non_trivially_copyable_array_t array2;
            [[maybe_unused]] auto out = ranges::uninitialized_copy(array1.begin(), array1.end(),
                                                                   array2.begin(), array2.end());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last == non_trivially_copyable_array_t{1, 2, 3});

        constexpr auto result_first_last_result = [] noexcept {
            non_trivially_copyable_array_t array1 = {3, 2, 1};
            non_trivially_copyable_array_t array2;
            ranges::uninitialized_copy(array1.begin(), array1.end(), array2.begin());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last_result == non_trivially_copyable_array_t{3, 2, 1});

        constexpr auto result_range = [] noexcept {
            non_trivially_copyable_array_t array1;
            non_trivially_copyable_array_t array2 = {4, 5, 6};
            [[maybe_unused]] auto          out    = ranges::uninitialized_copy(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == non_trivially_copyable_array_t{4, 5, 6});
    }

    SECTION("non trivially copyable at run-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_copyable_array_t::iterator,
                                           non_trivially_copyable_array_t::iterator>);
        {
            non_trivially_copyable_array_t array1 = {1, 2, 3};
            non_trivially_copyable_array_t array2;

            auto [it1, it2] = ranges::uninitialized_copy(array1.begin(), array1.end(),
                                                         array2.begin(), array2.end());
            CHECK(array1.end() == it1);
            CHECK(array2.end() == it2);
            CHECK(array2 == non_trivially_copyable_array_t{1, 2, 3});
        }
        {
            non_trivially_copyable_array_t array1 = {3, 2, 1};
            non_trivially_copyable_array_t array2;

            auto it = ranges::uninitialized_copy(array1.begin(), array1.end(), array2.begin());
            CHECK(array2.end() == it);
            CHECK(array2 == non_trivially_copyable_array_t{3, 2, 1});
        }
        {
            non_trivially_copyable_array_t array1;
            non_trivially_copyable_array_t array2 = {4, 5, 6};

            auto [it1, it2] = ranges::uninitialized_copy(array2, array1);
            CHECK(array1.end() == it2);
            CHECK(array2.end() == it1);
            CHECK(array1 == non_trivially_copyable_array_t{4, 5, 6});
        }
    }
}

TEST_CASE("fou::ranges::uninitialized_copy_no_overlap", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially copyable at compile-time (memcpy initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        constexpr auto result_first_last = [] noexcept {
            trivially_copyable_array_t array1 = {{{1}, {2}, {3}}};
            trivially_copyable_array_t array2;
            [[maybe_unused]] auto      out = ranges::uninitialized_copy_no_overlap(
                    array1.begin(), array1.end(), array2.begin(), array2.end());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last == trivially_copyable_array_t{{{1}, {2}, {3}}});

        constexpr auto result_first_last_result = [] noexcept {
            trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
            trivially_copyable_array_t array2;
            ranges::uninitialized_copy_no_overlap(array1.begin(), array1.end(), array2.begin());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last_result == trivially_copyable_array_t{{{3}, {2}, {1}}});

        constexpr auto result_range = [] noexcept {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};
            [[maybe_unused]] auto      out = ranges::uninitialized_copy_no_overlap(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == trivially_copyable_array_t{{{4}, {5}, {6}}});
    }

    SECTION("trivially copyable at run-time (memcpy initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        {
            trivially_copyable_array_t array1 = {{{1}, {2}, {3}}};
            trivially_copyable_array_t array2;

            auto [it1, it2] = ranges::uninitialized_copy_no_overlap(array1.begin(), array1.end(),
                                                                    array2.begin(), array2.end());
            CHECK(array1.end() == it1);
            CHECK(array2.end() == it2);
            CHECK(array2 == trivially_copyable_array_t{{{1}, {2}, {3}}});
        }
        {
            trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
            trivially_copyable_array_t array2;

            auto it = ranges::uninitialized_copy_no_overlap(array1.begin(), array1.end(),
                                                            array2.begin());
            CHECK(array2.end() == it);
            CHECK(array2 == trivially_copyable_array_t{{{3}, {2}, {1}}});
        }
        {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};

            auto [it1, it2] = ranges::uninitialized_copy_no_overlap(array2, array1);
            CHECK(array1.end() == it2);
            CHECK(array2.end() == it1);
            CHECK(array1 == trivially_copyable_array_t{{{4}, {5}, {6}}});
        }
    }

    SECTION("non trivially copyable at compile-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_copyable_array_t::iterator,
                                           non_trivially_copyable_array_t::iterator>);
        constexpr auto result_first_last = [] noexcept {
            non_trivially_copyable_array_t array1 = {1, 2, 3};
            non_trivially_copyable_array_t array2;
            [[maybe_unused]] auto          out = ranges::uninitialized_copy_no_overlap(
                    array1.begin(), array1.end(), array2.begin(), array2.end());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last == non_trivially_copyable_array_t{1, 2, 3});

        constexpr auto result_first_last_result = [] noexcept {
            non_trivially_copyable_array_t array1 = {3, 2, 1};
            non_trivially_copyable_array_t array2;
            ranges::uninitialized_copy_no_overlap(array1.begin(), array1.end(), array2.begin());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last_result == non_trivially_copyable_array_t{3, 2, 1});

        constexpr auto result_range = [] noexcept {
            non_trivially_copyable_array_t array1;
            non_trivially_copyable_array_t array2 = {4, 5, 6};
            [[maybe_unused]] auto out = ranges::uninitialized_copy_no_overlap(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == non_trivially_copyable_array_t{4, 5, 6});
    }

    SECTION("non trivially copyable at run-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_copyable_array_t::iterator,
                                           non_trivially_copyable_array_t::iterator>);
        {
            non_trivially_copyable_array_t array1 = {1, 2, 3};
            non_trivially_copyable_array_t array2;

            auto [it1, it2] = ranges::uninitialized_copy_no_overlap(array1.begin(), array1.end(),
                                                                    array2.begin(), array2.end());
            CHECK(array1.end() == it1);
            CHECK(array2.end() == it2);
            CHECK(array2 == non_trivially_copyable_array_t{1, 2, 3});
        }
        {
            non_trivially_copyable_array_t array1 = {3, 2, 1};
            non_trivially_copyable_array_t array2;

            auto it = ranges::uninitialized_copy_no_overlap(array1.begin(), array1.end(),
                                                            array2.begin());
            CHECK(array2.end() == it);
            CHECK(array2 == non_trivially_copyable_array_t{3, 2, 1});
        }
        {
            non_trivially_copyable_array_t array1;
            non_trivially_copyable_array_t array2 = {4, 5, 6};

            auto [it1, it2] = ranges::uninitialized_copy_no_overlap(array2, array1);
            CHECK(array1.end() == it2);
            CHECK(array2.end() == it1);
            CHECK(array1 == non_trivially_copyable_array_t{4, 5, 6});
        }
    }
}

TEST_CASE("fou::ranges::uninitialized_copy_n", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially copyable at compile-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        constexpr auto result_first_last = [] noexcept {
            trivially_copyable_array_t array1 = {{{1}, {2}, {3}}};
            trivially_copyable_array_t array2;
            ranges::uninitialized_copy_n(array1.begin(), array2.size(), array2.begin());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last == trivially_copyable_array_t{{{1}, {2}, {3}}});
    }

    SECTION("trivially copyable at run-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
        trivially_copyable_array_t array2;

        auto it = ranges::uninitialized_copy_n(array1.begin(), array2.size(), array2.begin());
        CHECK(array2.end() == it);
        CHECK(array2 == trivially_copyable_array_t{{{3}, {2}, {1}}});
    }

    SECTION("non trivially copyable at compile-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_copyable_array_t::iterator,
                                           non_trivially_copyable_array_t::iterator>);
        constexpr auto result_first_last = [] noexcept {
            non_trivially_copyable_array_t array1 = {1, 2, 3};
            non_trivially_copyable_array_t array2;
            ranges::uninitialized_copy_n(array1.begin(), array2.size(), array2.begin());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last == non_trivially_copyable_array_t{1, 2, 3});
    }

    SECTION("non trivially copyable at run-time") {
        using non_trivially_copyable_array_t = ::std::array<non_trivially_copyable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_copyable_array_t::iterator,
                                           non_trivially_copyable_array_t::iterator>);
        non_trivially_copyable_array_t array1 = {3, 2, 1};
        non_trivially_copyable_array_t array2;

        auto it = ranges::uninitialized_copy_n(array1.begin(), array2.size(), array2.begin());
        CHECK(array2.end() == it);
        CHECK(array2 == non_trivially_copyable_array_t{3, 2, 1});
    }
}

TEST_CASE("fou::ranges::uninitialized_move", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially movable at compile-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        constexpr auto result_range = [] noexcept {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};
            [[maybe_unused]] auto      out    = ranges::uninitialized_move(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == trivially_copyable_array_t{{{4}, {5}, {6}}});
    }

    SECTION("trivially movable at run-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        {
            trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
            trivially_copyable_array_t array2;

            auto it = ranges::uninitialized_move(array1.begin(), array1.end(), array2.begin());
            CHECK(array2.end() == it);
            CHECK(array2 == trivially_copyable_array_t{{{3}, {2}, {1}}});
        }
        {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};

            auto [it1, it2] = ranges::uninitialized_move(array2, array1);
            CHECK(array1.end() == it2);
            CHECK(array2.end() == it1);
            CHECK(array1 == trivially_copyable_array_t{{{4}, {5}, {6}}});
        }
    }

    SECTION("non trivially movable at compile-time") {
        using non_trivially_movable_array_t = ::std::array<non_trivially_movable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_movable_array_t::iterator,
                                           non_trivially_movable_array_t::iterator>);
        constexpr auto result_range = [] noexcept {
            non_trivially_movable_array_t array1;
            non_trivially_movable_array_t array2 = {4, 5, 6};
            [[maybe_unused]] auto         out    = ranges::uninitialized_move(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == non_trivially_movable_array_t{4, 5, 6});
    }

    SECTION("non trivially movable at run-time") {
        using non_trivially_movable_array_t = ::std::array<non_trivially_movable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_movable_array_t::iterator,
                                           non_trivially_movable_array_t::iterator>);
        {
            non_trivially_movable_array_t array1 = {3, 2, 1};
            non_trivially_movable_array_t array2;

            auto it = ranges::uninitialized_move(array1.begin(), array1.end(), array2.begin());
            CHECK(array2.end() == it);
            CHECK(array2 == non_trivially_movable_array_t{3, 2, 1});
        }
        {
            non_trivially_movable_array_t array1;
            non_trivially_movable_array_t array2 = {4, 5, 6};

            auto [it1, it2] = ranges::uninitialized_move(array2, array1);
            CHECK(array1.end() == it2);
            CHECK(array2.end() == it1);
            CHECK(array1 == non_trivially_movable_array_t{4, 5, 6});
        }
    }
}

TEST_CASE("fou::ranges::uninitialized_move_n", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially movable at compile-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        constexpr auto result_first_last = [] noexcept {
            trivially_copyable_array_t array1 = {{{1}, {2}, {3}}};
            trivially_copyable_array_t array2;
            ranges::uninitialized_move_n(array1.begin(), array2.size(), array2.begin());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last == trivially_copyable_array_t{{{1}, {2}, {3}}});
    }

    SECTION("trivially movable at run-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
        trivially_copyable_array_t array2;

        auto it = ranges::uninitialized_move_n(array1.begin(), array2.size(), array2.begin());
        CHECK(array2.end() == it);
        CHECK(array2 == trivially_copyable_array_t{{{3}, {2}, {1}}});
    }

    SECTION("non trivially movable at compile-time") {
        using non_trivially_movable_array_t = ::std::array<non_trivially_movable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_movable_array_t::iterator,
                                           non_trivially_movable_array_t::iterator>);
        constexpr auto result_first_last = [] noexcept {
            non_trivially_movable_array_t array1 = {1, 2, 3};
            non_trivially_movable_array_t array2;
            ranges::uninitialized_move_n(array1.begin(), array2.size(), array2.begin());
            return array2;
        }();
        STATIC_REQUIRE(result_first_last == non_trivially_movable_array_t{1, 2, 3});
    }

    SECTION("non trivially movable at run-time") {
        using non_trivially_movable_array_t = ::std::array<non_trivially_movable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_movable_array_t::iterator,
                                           non_trivially_movable_array_t::iterator>);
        non_trivially_movable_array_t array1 = {3, 2, 1};
        non_trivially_movable_array_t array2;

        auto it = ranges::uninitialized_move_n(array1.begin(), array2.size(), array2.begin());
        CHECK(array2.end() == it);
        CHECK(array2 == non_trivially_movable_array_t{3, 2, 1});
    }
}

TEST_CASE("fou::ranges::uninitialized_relocate", "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially relocatable at compile-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        constexpr auto result_range = [] noexcept {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};
            [[maybe_unused]] auto      out    = ranges::uninitialized_relocate(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == trivially_copyable_array_t{{{4}, {5}, {6}}});
    }

    SECTION("trivially relocatable at run-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        {
            trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
            trivially_copyable_array_t array2;

            auto it = ranges::uninitialized_relocate(array1.begin(), array1.end(), array2.begin());
            CHECK(array2.end() == it);
            CHECK(array2 == trivially_copyable_array_t{{{3}, {2}, {1}}});
        }
        {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};

            auto [it1, it2] = ranges::uninitialized_relocate(array2, array1);
            CHECK(array1.end() == it2);
            CHECK(array2.end() == it1);
            CHECK(array1 == trivially_copyable_array_t{{{4}, {5}, {6}}});
        }
    }

    SECTION("non trivially relocatable at compile-time") {
        using non_trivially_movable_array_t = ::std::array<non_trivially_movable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_movable_array_t::iterator,
                                           non_trivially_movable_array_t::iterator>);
        constexpr auto result_range = [] noexcept {
            non_trivially_movable_array_t array1;
            non_trivially_movable_array_t array2 = {4, 5, 6};
            [[maybe_unused]] auto         out    = ranges::uninitialized_relocate(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == non_trivially_movable_array_t{4, 5, 6});
    }

    SECTION("non trivially relocatable at run-time") {
        relocatable_counter_t::reset();

        using InputIterator  = int*;
        using OutputIterator = relocatable_counter_t*;

        int const N         = 5;
        int       values[N] = {1, 2, 3, 4, 5};

        alignas(relocatable_counter_t) std::byte pool[sizeof(relocatable_counter_t) * N];
        relocatable_counter_t*                   counted = (relocatable_counter_t*)pool;

        // clang-format off
        auto result = ranges::uninitialized_relocate(InputIterator(values), InputIterator(values + 1),
                                                     OutputIterator(counted));
        CHECK(result == OutputIterator(counted + 1));
        CHECK(relocatable_counter_t::constructed == 1);
        CHECK(relocatable_counter_t::count == 1);
        CHECK(counted[0].value == 1);
        CHECK(values[0] == 0);

        result = ranges::uninitialized_relocate( InputIterator(values  + 1),  InputIterator(values  + N),
                                                OutputIterator(counted + 1), OutputIterator(counted + N)).out;
        CHECK(result == OutputIterator(counted + N));
        CHECK(relocatable_counter_t::count == 5);
        CHECK(relocatable_counter_t::constructed == 5);
        CHECK(counted[1].value == 2);
        CHECK(counted[2].value == 3);
        CHECK(counted[3].value == 4);
        CHECK(counted[4].value == 5);
        CHECK(values[1] == 0);
        CHECK(values[2] == 0);
        CHECK(values[3] == 0);
        CHECK(values[4] == 0);
        // clang-format on

        destroy_range(counted, counted + N);
        CHECK(relocatable_counter_t::count == 0);
    }
}

TEST_CASE("fou::ranges::uninitialized_relocate_no_overlap",
          "[memory/uninitialized_algorithms.hpp]") {
    using namespace flux::fou;

    SECTION("trivially relocatable at compile-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        constexpr auto result_range = [] noexcept {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};
            [[maybe_unused]] auto out = ranges::uninitialized_relocate_no_overlap(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == trivially_copyable_array_t{{{4}, {5}, {6}}});
    }

    SECTION("trivially relocatable at run-time (memmove initialization)") {
        using trivially_copyable_array_t = ::std::array<trivially_copyable_t, 3>;
        static_assert(meta::memcpyable<trivially_copyable_array_t::iterator,
                                       trivially_copyable_array_t::iterator>);
        {
            trivially_copyable_array_t array1 = {{{3}, {2}, {1}}};
            trivially_copyable_array_t array2;

            auto it = ranges::uninitialized_relocate_no_overlap(array1.begin(), array1.end(),
                                                                array2.begin());
            CHECK(array2.end() == it);
            CHECK(array2 == trivially_copyable_array_t{{{3}, {2}, {1}}});
        }
        {
            trivially_copyable_array_t array1;
            trivially_copyable_array_t array2 = {{{4}, {5}, {6}}};

            auto [it1, it2] = ranges::uninitialized_relocate_no_overlap(array2, array1);
            CHECK(array1.end() == it2);
            CHECK(array2.end() == it1);
            CHECK(array1 == trivially_copyable_array_t{{{4}, {5}, {6}}});
        }
    }

    SECTION("non trivially relocatable at compile-time") {
        using non_trivially_movable_array_t = ::std::array<non_trivially_movable_t, 3>;
        static_assert(not meta::memcpyable<non_trivially_movable_array_t::iterator,
                                           non_trivially_movable_array_t::iterator>);
        constexpr auto result_range = [] noexcept {
            non_trivially_movable_array_t array1;
            non_trivially_movable_array_t array2 = {4, 5, 6};
            [[maybe_unused]] auto out = ranges::uninitialized_relocate_no_overlap(array2, array1);
            return array1;
        }();
        STATIC_REQUIRE(result_range == non_trivially_movable_array_t{4, 5, 6});
    }

    SECTION("non trivially relocatable at run-time") {
        relocatable_counter_t::reset();

        using InputIterator  = int*;
        using OutputIterator = relocatable_counter_t*;

        int const N         = 5;
        int       values[N] = {1, 2, 3, 4, 5};

        alignas(relocatable_counter_t) std::byte pool[sizeof(relocatable_counter_t) * N];
        relocatable_counter_t*                   counted = (relocatable_counter_t*)pool;

        auto result = ranges::uninitialized_relocate_no_overlap(
                InputIterator(values), InputIterator(values + 1), OutputIterator(counted));
        CHECK(result == OutputIterator(counted + 1));
        CHECK(relocatable_counter_t::constructed == 1);
        CHECK(relocatable_counter_t::count == 1);
        CHECK(counted[0].value == 1);
        CHECK(values[0] == 0);

        result = ranges::uninitialized_relocate_no_overlap(
                         InputIterator(values + 1), InputIterator(values + N),
                         OutputIterator(counted + 1), OutputIterator(counted + N))
                         .out;
        CHECK(result == OutputIterator(counted + N));
        CHECK(relocatable_counter_t::count == 5);
        CHECK(relocatable_counter_t::constructed == 5);
        CHECK(counted[1].value == 2);
        CHECK(counted[2].value == 3);
        CHECK(counted[3].value == 4);
        CHECK(counted[4].value == 5);
        CHECK(values[1] == 0);
        CHECK(values[2] == 0);
        CHECK(values[3] == 0);
        CHECK(values[4] == 0);

        destroy_range(counted, counted + N);
        CHECK(relocatable_counter_t::count == 0);
    }
}