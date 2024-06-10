#include <flux/foundation.hpp>

#include <catch2/catch.hpp>

TEST_CASE("fou::vector", "[flux-containers/vector.hpp]") {
    using int_vector = ::flux::fou::vector<int>;
    static_assert(sizeof(int*) * 3 == sizeof(int_vector));

    SECTION("default constructor") {
        int_vector v;
        CHECK(v.empty());
        CHECK(v.size() == 0);
        CHECK(v.capacity() == 0);
    }

    SECTION("count constructor") {
        {
            int_vector v(5);
            CHECK_FALSE(v.empty());
            CHECK(v.size() == 5);
            CHECK(v.capacity() >= 5);
        }
        {
            int_vector v(3, 2024);
            CHECK_FALSE(v.empty());
            CHECK(v.size() == 3);
            CHECK(v.capacity() >= 3);

            CHECK(v[0] == 2024);
            CHECK(v[1] == 2024);
            CHECK(v[2] == 2024);
        }
    }

    SECTION("assign") {
        {
            int_vector v(3, 5);
            v.assign(5, 2024);
            CHECK(v.size() == 5);
            CHECK(v.capacity() >= 5);

            CHECK(v[0] == 2024);
            CHECK(v[1] == 2024);
            CHECK(v[2] == 2024);
            CHECK(v[3] == 2024);
            CHECK(v[4] == 2024);
        }
        {
            int_vector v(5, 3);
            v.assign(3, 2024);
            CHECK(v.size() == 3);
            CHECK(v.capacity() >= 3);

            CHECK(v[0] == 2024);
            CHECK(v[1] == 2024);
            CHECK(v[2] == 2024);
        }
    }

    SECTION("resize") {
        {
            int_vector v(3, 2024);
            v.resize(5, 3);
            CHECK(v.size() == 5);
            CHECK(v.capacity() >= 5);

            CHECK(v[0] == 2024);
            CHECK(v[1] == 2024);
            CHECK(v[2] == 2024);
            CHECK(v[3] == 3);
            CHECK(v[4] == 3);
        }
        {
            int_vector v(5, 3);
            v.resize(3, 2024);
            CHECK(v.size() == 3);
            CHECK(v.capacity() >= 3);

            CHECK(v[0] == 3);
            CHECK(v[1] == 3);
            CHECK(v[2] == 3);
        }
    }
}