#include <flux/foundation.hpp>

#include <catch2/catch.hpp>

using namespace flux;

struct A {
    int i;
};

struct B {
    A     a;
    float f;
    char  c;
};

TEST_CASE("fou::construct_in_place", "[memory/construct.hpp]") {
    SECTION("construct in place lvalue") {
        alignas(A) std::byte buffer[sizeof(A)];
        A*                   object = (A*)buffer;

        int expected_value = 2024;
        fou::construct_in_place(object, expected_value);
        CHECK(object->i == expected_value);

        fou::destroy_in_place(object);
    }

    SECTION("construct in place rvalue") {
        alignas(A) std::byte buffer[sizeof(A)];
        A*                   object = (A*)buffer;

        fou::construct_in_place(object, 2025);
        CHECK(object->i == 2025);

        fou::destroy_in_place(object);
    }

    SECTION("construct in place variadic") {
        alignas(B) std::byte buffer[sizeof(B)];
        B*                   object = (B*)buffer;

        fou::construct_in_place(object, A{10}, 3.14f, 'w');
        CHECK(object->a.i == 10);
        CHECK(object->f == 3.14f);
        CHECK(object->c == 'w');

        fou::destroy_in_place(object);
    }
}