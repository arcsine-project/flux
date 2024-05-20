#include <flux/foundation.hpp>

#include <catch2/catch.hpp>

struct dummy {
    int value{};

    constexpr dummy() noexcept {}
    constexpr dummy(int v) noexcept : value{v} {}

    constexpr dummy(dummy const&) noexcept {}
    constexpr dummy(dummy&&) noexcept {}

    constexpr dummy& operator=(dummy const&) noexcept {
        return *this;
    }
    constexpr dummy& operator=(dummy&&) noexcept {
        return *this;
    }

    constexpr ~dummy() {}
};

using namespace flux;
static_assert(not meta::trivial<fou::uninitialized_storage<int>>);
static_assert(meta::standard_layout<fou::uninitialized_storage<int>>);
static_assert(not meta::trivially_default_constructible<fou::uninitialized_storage<int>>);
static_assert(meta::trivially_copyable<fou::uninitialized_storage<int>>);
static_assert(meta::trivially_copy_constructible<fou::uninitialized_storage<int>>);
static_assert(meta::trivially_move_constructible<fou::uninitialized_storage<int>>);
static_assert(meta::trivially_copy_assignable<fou::uninitialized_storage<int>>);
static_assert(meta::trivially_move_assignable<fou::uninitialized_storage<int>>);
static_assert(meta::trivially_destructible<fou::uninitialized_storage<int>>);

static_assert(not meta::trivial<fou::uninitialized_storage<dummy>>);
static_assert(meta::standard_layout<fou::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_default_constructible<fou::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copyable<fou::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copy_constructible<fou::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_move_constructible<fou::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copy_assignable<fou::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_move_assignable<fou::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_destructible<fou::uninitialized_storage<dummy>>);

consteval int storage_construct() noexcept {
    fou::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    fou::detail::construct_at(storage.data(), 10);
    int value = storage.data()->value;
    fou::detail::destroy_at(storage.data());
    return value;
}

consteval int storage_accessors() noexcept {
    fou::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    fou::detail::construct_at(get<dummy*>(storage), 20);
    int value = get<dummy&>(storage).value;
    fou::detail::destroy_at(get<dummy*>(storage));
    return value;
}

TEST_CASE("fou::uninitialized_storage", "[memory/uninitialized_storage.hpp]") {
    SECTION("evaluate in constant expression") {
        STATIC_REQUIRE(10 == storage_construct());
        STATIC_REQUIRE(20 == storage_accessors());
    }

    SECTION("construct from fundamental type") {
        fou::uninitialized_storage<int> storage;

        int* value = fou::detail::construct_at(storage.data(), 5);
        CHECK(5 == *value);
        CHECK(get<int&>(*value) == get<int&>(storage));
        CHECK(*get<int*>(*value) == *get<int*>(storage));
        fou::detail::destroy_at(storage.data());
    }

    SECTION("construct from user-defined type") {
        fou::uninitialized_storage<dummy> storage;

        fou::detail::construct_at(storage.data(), 8);
        CHECK(8 == get<dummy&>(storage).value);
        CHECK(8 == get<dummy*>(storage)->value);
        fou::detail::destroy_at(storage.data());
    }
}