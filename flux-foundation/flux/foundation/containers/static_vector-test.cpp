#include <flux/foundation.hpp>

#include <catch2/catch.hpp>

namespace trivially_copyable_vector {

using vector_type = flux::fou::static_vector<int, 5>;

static_assert(not flux::meta::trivial<vector_type>);
static_assert(flux::meta::trivially_copyable<vector_type>);
static_assert(flux::meta::standard_layout<vector_type>);

} // namespace trivially_copyable_vector

struct copyable_or_movable {
    copyable_or_movable() = default;

    constexpr copyable_or_movable(copyable_or_movable const& other) noexcept = delete;
    constexpr copyable_or_movable(copyable_or_movable&& other) noexcept      = delete;

    constexpr copyable_or_movable& operator=(copyable_or_movable const& other) noexcept = delete;
    constexpr copyable_or_movable& operator=(copyable_or_movable&& other) noexcept      = delete;
};

namespace copyable_or_movable_vector {

using vector_type = flux::fou::static_vector<copyable_or_movable, 5>;

static_assert(not flux::meta::trivially_copyable<vector_type>);
static_assert(flux::meta::copy_assignable<vector_type>);
static_assert(flux::meta::copy_constructible<vector_type>);
static_assert(flux::meta::move_assignable<vector_type>);
static_assert(flux::meta::trivially_destructible<vector_type>);

} // namespace copyable_or_movable_vector

struct nontrivial_int {
    int x;

    constexpr nontrivial_int() noexcept : x{0} {}

    constexpr nontrivial_int(int val) noexcept : x{val} {}
    constexpr ~nontrivial_int() {}

    constexpr nontrivial_int(nontrivial_int const& other) noexcept {
        x = other.x;
    }
    constexpr nontrivial_int(nontrivial_int&& other) noexcept {
        x = other.x;
    }

    constexpr nontrivial_int& operator=(nontrivial_int const& other) noexcept {
        x = other.x;
        return *this;
    }
    constexpr nontrivial_int& operator=(nontrivial_int&& other) noexcept {
        x = other.x;
        return *this;
    }

    constexpr bool operator==(nontrivial_int const& other) const noexcept {
        return x == other.x;
    }
};

namespace not_trivially_copyable_vector {

using vector_type = flux::fou::static_vector<nontrivial_int, 5>;

static_assert(not flux::meta::trivially_copyable<vector_type>);
static_assert(not flux::meta::trivially_copy_constructible<vector_type>);
static_assert(not flux::meta::trivially_copy_assignable<vector_type>);
static_assert(not flux::meta::trivially_move_assignable<vector_type>);
static_assert(not flux::meta::trivially_destructible<vector_type>);

} // namespace not_trivially_copyable_vector

struct nontrivial_counter {
    int        x;
    bool       has_resource;
    static int ctor_dtor_call_counter;

    nontrivial_counter() noexcept : x{}, has_resource{true} {
        ++ctor_dtor_call_counter;
    }

    nontrivial_counter(int val) noexcept : x{val}, has_resource{true} {
        ++ctor_dtor_call_counter;
    }

    ~nontrivial_counter() {
        if (has_resource) {
            --ctor_dtor_call_counter;
        }
    }

    nontrivial_counter(nontrivial_counter const& other) noexcept {
        has_resource = other.has_resource;
        x            = other.x;
        if (has_resource) {
            ++ctor_dtor_call_counter;
        }
    }
    nontrivial_counter(nontrivial_counter&& other) noexcept {
        has_resource = other.has_resource;
        x            = other.x;
        if (other.has_resource) {
            other.has_resource = false;
        }
    }

    nontrivial_counter& operator=(nontrivial_counter const& other) noexcept {
        x = other.x;
        if (has_resource) {
            has_resource = false;
        }
        if (other.has_resource) {
            has_resource = true;
        }
        return *this;
    }
    nontrivial_counter& operator=(nontrivial_counter&& other) noexcept {
        x = other.x;
        if (has_resource) {
            has_resource = false;
        }
        has_resource = std::exchange(other.has_resource, false);
        return *this;
    }

    bool operator==(nontrivial_counter const& other) const noexcept {
        return x == other.x && has_resource == other.has_resource;
    }
};
int nontrivial_counter::ctor_dtor_call_counter = 0;

struct complex_type {
    using int_array = ::std::array<int, 2>;

    constexpr complex_type(int param_a, int param_b1, int param_b2, int param_c)
            : a(param_a), b({param_b1, param_b2}), c(param_c) {}

    int       a;
    int_array b;
    int       c;
};

TEST_CASE("fou::static_vector", "[flux-containers/static_vector.hpp]") {
    using namespace flux;

    SECTION("default constructor") {
        {
            constexpr fou::static_vector<int, 5> v{};
            STATIC_REQUIRE(v.empty());
            STATIC_REQUIRE(v.capacity() == 5);
            STATIC_REQUIRE(decltype(v)::max_size() == 5);
        }
        {
            fou::static_vector<int, 5> v{};
            CHECK(v.empty());
            CHECK(v.max_size() == 5);
            CHECK(v.capacity() == 5);
        }
    }

    SECTION("count constructor") {
        {
            fou::static_vector<int, 5> v(5);
            CHECK_FALSE(v.empty());
            CHECK(v.capacity() == 5);
            STATIC_REQUIRE(decltype(v)::max_size() == 5);
            CHECK(v == fou::static_vector{0, 0, 0, 0, 0});
        }
        {
            fou::static_vector<int, 5> v(5, 3);
            CHECK_FALSE(v.empty());
            CHECK(v.capacity() == 5);
            STATIC_REQUIRE(decltype(v)::max_size() == 5);
            CHECK(v == fou::static_vector{3, 3, 3, 3, 3});
        }
    }

    SECTION("list constructor") {
        fou::static_vector<int, 5> v{1, 2, 3};
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 3);
        CHECK(v.size() == 3);
        CHECK(v.capacity() == 5);
    }

    SECTION("range constructor") {
        {
            constexpr fou::static_vector<int, 3> v1{7, 9};
            STATIC_REQUIRE(v1[0] == 7);
            STATIC_REQUIRE(v1[1] == 9);
            STATIC_REQUIRE(v1.size() == 2);

            constexpr fou::static_vector<int, 5> v2{v1.begin(), v1.end()};
            STATIC_REQUIRE(v2[0] == 7);
            STATIC_REQUIRE(v2[1] == 9);
            STATIC_REQUIRE(v2.size() == 2);
        }
        {
            fou::static_vector<int, 3> v1{2, 1};
            CHECK(v1[0] == 2);
            CHECK(v1[1] == 1);
            CHECK(v1.size() == 2);

            fou::static_vector<int, 5> v2{v1.begin(), v1.end()};
            CHECK(v2[0] == 2);
            CHECK(v2[1] == 1);
            CHECK(v2.size() == 2);
        }
    }

    SECTION("copy constructor") {
        {
            constexpr auto v1 = [] {
                fou::static_vector<nontrivial_int, 3> v;
                v.emplace_back(1);
                v.emplace_back(2);
                return v;
            }();
            constexpr fou::static_vector<nontrivial_int, 3> v2{v1};
            STATIC_REQUIRE(v1 == v2);
        }
        {
            fou::static_vector<nontrivial_int, 3> v1;
            v1.emplace_back(1);
            v1.emplace_back(2);

            fou::static_vector<nontrivial_int, 3> v2{v1};
            CHECK(v1 == v2);
        }
    }

    SECTION("move constructor") {
        {
            constexpr auto v1 = [] {
                fou::static_vector<nontrivial_int, 3> v;
                v.emplace_back(1);
                v.emplace_back(2);
                return v;
            }();
            constexpr fou::static_vector<nontrivial_int, 3> v2{std::move(v1)};
            STATIC_REQUIRE(v2 == fou::static_vector{nontrivial_int(1), nontrivial_int(2)});
        }
        {
            fou::static_vector<nontrivial_int, 3> v1;
            v1.emplace_back(1);
            v1.emplace_back(2);

            fou::static_vector<nontrivial_int, 3> v2{std::move(v1)};
            CHECK(v2 == fou::static_vector{nontrivial_int(1), nontrivial_int(2)});
        }
        {
            fou::static_vector<int, 5> v1{5, 6, 7};
            fou::static_vector<int, 5> v2{std::move(v1)};
            CHECK(v2 == fou::static_vector{5, 6, 7});
        }
    }

    SECTION("copy assignment") {
        {
            constexpr auto cmp = [] {
                fou::static_vector<nontrivial_int, 3> v1;
                v1.emplace_back(1);
                v1.emplace_back(2);
                fou::static_vector<nontrivial_int, 3> v2{};
                v2 = v1;
                return v1 == v2;
            }();
            STATIC_REQUIRE(cmp);
        }
        {
            fou::static_vector<nontrivial_int, 3> v1;
            v1.emplace_back(1);
            v1.emplace_back(2);

            fou::static_vector<nontrivial_int, 3> v2;
            v2 = v1;
            CHECK(v1 == v2);
        }
    }

    SECTION("move assignment") {
        {
            constexpr auto cmp = [] {
                fou::static_vector<nontrivial_int, 3> v1;
                v1.emplace_back(1);
                v1.emplace_back(2);

                fou::static_vector<nontrivial_int, 3> v2{};
                v2 = std::move(v1);

                return v2 == fou::static_vector{nontrivial_int(1), nontrivial_int(2)};
            }();
            STATIC_REQUIRE(cmp);
        }
        {
            fou::static_vector<nontrivial_int, 3> v1;
            v1.emplace_back(1);
            v1.emplace_back(2);

            fou::static_vector<nontrivial_int, 3> v2;
            v2 = std::move(v1);
            CHECK(v2 == fou::static_vector{nontrivial_int(1), nontrivial_int(2)});
        }
        {
            fou::static_vector<int, 5> v1{1, 2};
            fou::static_vector<int, 5> v2;
            v2 = std::move(v1);
            CHECK(v2 == fou::static_vector{1, 2});
        }
    }

    SECTION("equality operator") {
        constexpr fou::static_vector v{1, 2, 3};
        STATIC_REQUIRE(v == fou::static_vector{1, 2, 3});
        STATIC_REQUIRE(v != fou::static_vector{1, 2, 1});
        STATIC_REQUIRE(v != fou::static_vector{1, 2, 3, 4});
    }

    SECTION("compare") {
        {
            constexpr fou::static_vector v1{1, 2, 3};
            constexpr fou::static_vector v2{1, 2, 4};
            STATIC_REQUIRE(v1 < v2);
            STATIC_REQUIRE(v1 <= v2);
            STATIC_REQUIRE_FALSE(v1 > v2);
            STATIC_REQUIRE_FALSE(v1 >= v2);
        }
        {
            fou::static_vector v1{1, 2, 3};
            fou::static_vector v2{1, 2, 4};
            CHECK(v1 < v2);
            CHECK(v1 <= v2);
            CHECK_FALSE(v1 > v2);
            CHECK_FALSE(v1 >= v2);
        }
        {
            constexpr fou::static_vector v1{1, 5};
            constexpr fou::static_vector v2{1, 2, 4};
            STATIC_REQUIRE_FALSE(v1 == v2);
            STATIC_REQUIRE(v1 != v2);
            STATIC_REQUIRE_FALSE(v1 < v2);
            STATIC_REQUIRE_FALSE(v1 <= v2);
            STATIC_REQUIRE(v1 > v2);
            STATIC_REQUIRE(v1 >= v2);
        }
        {
            fou::static_vector v1{1, 5};
            fou::static_vector v2{1, 2, 4};
            CHECK_FALSE(v1 == v2);
            CHECK(v1 != v2);
            CHECK_FALSE(v1 < v2);
            CHECK_FALSE(v1 <= v2);
            CHECK(v1 > v2);
            CHECK(v1 >= v2);
        }
        {
            constexpr fou::static_vector v1{1, 2, 3};
            constexpr fou::static_vector v2{1, 5};
            STATIC_REQUIRE(v1 < v2);
            STATIC_REQUIRE(v1 <= v2);
            STATIC_REQUIRE_FALSE(v1 > v2);
            STATIC_REQUIRE_FALSE(v1 >= v2);
        }
        {
            fou::static_vector v1{1, 2, 3};
            fou::static_vector v2{1, 2, 4};
            CHECK(v1 < v2);
            CHECK(v1 <= v2);
            CHECK_FALSE(v1 > v2);
            CHECK_FALSE(v1 >= v2);
        }
    }

    SECTION("assign") {
        {
            constexpr auto v1 = [] {
                fou::static_vector<int, 5> v;
                v.assign({1, 2, 3, 4, 5});
                return v;
            }();
            STATIC_REQUIRE_FALSE(v1.empty());
            STATIC_REQUIRE(v1 == fou::static_vector{1, 2, 3, 4, 5});
        }
        {
            fou::static_vector<int, 5> v;
            v.assign({1, 2, 3});
            CHECK_FALSE(v.empty());
            CHECK(v == fou::static_vector<int, 5>{1, 2, 3});
            v.assign(2, 5);
            CHECK(v == fou::static_vector<int, 5>{5, 5});
        }
        {
            fou::static_vector<nontrivial_int, 3> v1{10, 20, 30};
            fou::static_vector<nontrivial_int, 3> v2;
            v2.assign(v1.begin(), v1.end());
            CHECK(v1 == v2);
        }
    }

    SECTION("clear") {
        {
            constexpr auto v1 = [] {
                fou::static_vector<int, 5> v{1, 2, 3, 4, 5};
                v.clear();
                return v;
            }();
            STATIC_REQUIRE(v1.empty());
            STATIC_REQUIRE(v1.max_size() == 5);
            STATIC_REQUIRE(v1.capacity() == 5);
        }
        {
            fou::static_vector<int, 5> v{1, 2, 3, 4, 5};
            v.clear();
            CHECK(v.empty());
            CHECK(v.max_size() == 5);
            CHECK(v.capacity() == 5);
        }
        {
            fou::static_vector<nontrivial_counter, 3> v{1, 2, 3};
            v.clear();
            CHECK(v.empty());
            CHECK(v.max_size() == 3);
            CHECK(v.capacity() == 3);
        }
    }

    SECTION("resize") {
        {
            constexpr auto v1 = [] {
                fou::static_vector<int, 5> v{1, 2, 3, 4, 5};
                v.resize(3);
                return v;
            }();
            STATIC_REQUIRE(v1 == fou::static_vector<int, 5>{1, 2, 3});
            STATIC_REQUIRE(v1.capacity() == 5);
        }
        {
            constexpr auto v1 = [] {
                fou::static_vector<nontrivial_int, 5> v{1, 2};
                v.resize(4, 7);
                return v;
            }();
            STATIC_REQUIRE(v1 == fou::static_vector<nontrivial_int, 5>{1, 2, 7, 7});
            STATIC_REQUIRE(v1.capacity() == 5);
        }
        {
            fou::static_vector<int, 5> v{1, 2, 3, 4, 5};
            v.resize(3);
            CHECK(v == fou::static_vector<int, 5>{1, 2, 3});
        }
        {
            fou::static_vector<nontrivial_counter, 5> v{1, 2};

            auto const x = nontrivial_counter{7};
            v.resize(4, x);
            CHECK(v == fou::static_vector<nontrivial_counter, 5>{1, 2, 7, 7});
        }
    }

    SECTION("data") {
        fou::static_vector<int, 3> v{1, 2, 3};

        int* p = v.data();
        CHECK(p[0] == 1);
        CHECK(p[1] == 2);
        CHECK(p[2] == 3);

        auto const& cref_v = v;
        int const*  cp     = cref_v.data();
        CHECK(cp[0] == 1);
        CHECK(cp[1] == 2);
        CHECK(cp[2] == 3);
    }

    SECTION("front/back") {
        fou::static_vector<int, 3> v{1, 2, 3};
        CHECK(v.front() == 1);
        CHECK(v.back() == 3);

        auto const& cref_v = v;
        CHECK(cref_v.front() == 1);
        CHECK(cref_v.back() == 3);
    }

    SECTION("subsript operator") {
        fou::static_vector<int, 3> const v{1, 2, 3};
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 3);
        CHECK(v.cbegin() != v.cend());
    }

    SECTION("push_back") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 5> v{1, 2};
                v.push_back(5);
                int const value = 8;
                v.push_back(value);
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector<int, 5>{1, 2, 5, 8});
        }
        {
            fou::static_vector<int, 3> v;
            v.push_back(1);
            int const x = 2;
            v.push_back(x);
            v.push_back(3);
            CHECK_FALSE(v.empty());
            CHECK(v.size() == 3);
            CHECK(v.max_size() == 3);
            CHECK(v == fou::static_vector{1, 2, 3});
        }
    }

    SECTION("emplace_back") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 5> v{1, 2};
                v.emplace_back(5);
                int const value = 8;
                v.emplace_back(value);
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector<int, 5>{1, 2, 5, 8});
        }
        {
            fou::static_vector<complex_type, 3> v;
            v.emplace_back(1, 2, 3, 4);
            auto ref = v.emplace_back(101, 202, 303, 404);
            CHECK(ref.a == 101);
            CHECK(ref.c == 404);
        }
    }

    SECTION("reserve") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 6> v{};
                v.reserve(5);
                return v;
            }();
            STATIC_REQUIRE(result.max_size() == 6);
            STATIC_REQUIRE(result.capacity() == 6);
        }
        {
            fou::static_vector<int, 10> v;
            v.reserve(8);
            CHECK(v.max_size() == 10);
            CHECK(v.capacity() == 10);
        }
    }

    SECTION("pop_back") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 3> v{0, 1, 2};
                v.pop_back();
                return v;
            }();
            STATIC_REQUIRE(result.size() == 2);
            STATIC_REQUIRE(result.capacity() == 3);
            STATIC_REQUIRE(result == fou::static_vector<int, 3>{0, 1});
        }
        {
            fou::static_vector<nontrivial_counter, 4> v{5, 6, 7};
            v.pop_back();
            CHECK(v == fou::static_vector<nontrivial_counter, 4>{5, 6});
        }
    }

    SECTION("insert") {
        {
            fou::static_vector<int, 8> v;

            auto x  = 2;
            auto it = v.begin();
            it      = v.insert(it, 3);
            it      = v.insert(it, x);
            it      = v.insert(it, 1);
            CHECK(it == v.begin());
            it = v.insert(v.end(), {5, 6, 7});
            CHECK(it != v.begin());
            CHECK(v == fou::static_vector<int, 8>{1, 2, 3, 5, 6, 7});
            CHECK(v.size() == 6);
            CHECK(v.max_size() == 8);
        }
        {
            fou::static_vector<nontrivial_counter, 5> v;

            auto it = v.insert(v.end(), {5, 6, 7});
            it      = v.insert(v.begin(), 3);
            it      = v.insert(v.begin() + 2, 1);
            CHECK(v == fou::static_vector<nontrivial_counter, 5>{3, 5, 1, 6, 7});
        }
        {
            constexpr auto result = [] {
                fou::static_vector<int, 5> v{1, 2, 3};
                (void)v.insert(v.begin(), 5);
                int const value = 8;
                (void)v.insert(v.begin() + 2, value);
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector{5, 1, 8, 2, 3});
        }
    }

    SECTION("insert range") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 6> v{1, 2, 3, 4};
                ::std::array<int, 3>       a{5, 6, 7};
                (void)v.insert(v.begin() + 2, a.begin(), a.begin() + 2);
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector{1, 2, 5, 6, 3, 4});
        }
        {
            constexpr auto result = [] {
                fou::static_vector<nontrivial_int, 3> v;
                ::std::array<nontrivial_int, 3>       a{0, 1, 2};
                (void)v.insert(v.begin(), a.begin(), a.end());
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector<nontrivial_int, 3>{0, 1, 2});
        }
        {
            fou::static_vector<int, 5> v{0, 1, 2};
            ::std::array<int, 3>       a{5, 6, 7};

            auto it = v.insert(v.begin() + 1, a.begin(), a.begin() + 2);
            CHECK(it == v.begin() + 1);
            CHECK(v == fou::static_vector{0, 5, 6, 1, 2});
            CHECK(v.size() == 5);
        }
    }

    SECTION("emplace") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 5> v{0, 1, 2};
                (void)v.emplace(v.begin() + 1, 3);
                (void)v.emplace(v.begin() + 1, 4);
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector{0, 4, 3, 1, 2});
        }
        {
            constexpr auto result = [] {
                fou::static_vector<nontrivial_int, 3> v;
                (void)v.emplace(v.begin(), 3);
                (void)v.emplace(v.begin(), 4);
                (void)v.emplace(v.begin(), 2);
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector<nontrivial_int, 3>{2, 4, 3});
        }
        {
            fou::static_vector<nontrivial_counter, 5> v{0, 1, 2};

            auto it = v.begin() + 1;
            it      = v.emplace(it, 3);
            it      = v.emplace(it, 4);
            CHECK(it == v.begin() + 1);
            CHECK(v == fou::static_vector<nontrivial_counter, 5>{0, 4, 3, 1, 2});
        }
    }

    SECTION("erase one") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 3> v{2, 3, 4};
                (void)v.erase(v.begin());
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector<int, 3>{3, 4});
        }
        {
            fou::static_vector<nontrivial_counter, 3> v{2, 3, 4};
            auto                                      it = v.erase(v.begin());
            CHECK(it == v.begin());
            CHECK(v == fou::static_vector<nontrivial_counter, 3>{3, 4});
        }
    }

    SECTION("erase range") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 5> v{0, 1, 2, 3, 4};
                (void)v.erase(v.begin() + 1, v.begin() + 3);
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector<int, 5>{0, 3, 4});
        }
        {
            constexpr auto result = [] {
                fou::static_vector<nontrivial_int, 6> v{0, 1, 2};
                ::std::array<nontrivial_int, 3>       a{5, 6, 7};
                (void)v.insert(v.end(), a.begin(), a.end());
                (void)v.erase(v.begin() + 2, v.begin() + 4);
                return v;
            }();
            STATIC_REQUIRE(result == fou::static_vector<nontrivial_int, 6>{0, 1, 6, 7});
        }
        {
            fou::static_vector<int, 8> v1{2, 1, 4, 5, 0, 3};
            auto                       it = v1.erase(v1.begin() + 2, v1.begin() + 4);
            CHECK(it == v1.begin() + 2);
            CHECK(v1 == fou::static_vector<int, 8>{2, 1, 0, 3});
        }
    }

    SECTION("iterators") {
        {
            fou::static_vector<int, 5> v{0, 1, 2};

            int expected_value = 0;
            int count          = 0;
            std::ranges::for_each(v, [&](const int n) {
                CHECK(expected_value == n);
                ++expected_value, ++count;
            });
            CHECK(3 == count);

            expected_value = 0;
            count          = 0;
            std::for_each(v.cbegin(), v.cend(), [&](const int n) {
                CHECK(expected_value == n);
                ++expected_value, ++count;
            });
            CHECK(3 == count);
        }
        {
            fou::static_vector<int, 5> const v{3, 4, 5};

            int expected_value = 3;
            int count          = 0;
            std::ranges::for_each(v, [&](const int n) {
                CHECK(expected_value == n);
                ++expected_value, ++count;
            });
            CHECK(3 == count);
        }
        {
            fou::static_vector<int, 5> v{0, 1, 2};

            int expected_value = 2;
            int count          = 0;
            std::for_each(v.rbegin(), v.rend(), [&](const int n) {
                CHECK(expected_value == n);
                --expected_value, ++count;
            });
            CHECK(3 == count);

            expected_value = 2;
            count          = 0;
            std::for_each(v.crbegin(), v.crend(), [&](const int n) {
                CHECK(expected_value == n);
                --expected_value, ++count;
            });
            CHECK(3 == count);
        }
        {
            fou::static_vector<int, 5> const v{3, 4, 5};

            int expected_value = 5;
            int count          = 0;
            std::for_each(v.rbegin(), v.rend(), [&](const int n) {
                CHECK(expected_value == n);
                --expected_value, ++count;
            });
            CHECK(3 == count);
        }
    }

    SECTION("swap") {
        {
            constexpr auto result = [] {
                fou::static_vector<int, 5> v1{0, 1, 2};
                fou::static_vector<int, 5> v2{5, 6, 7};
                swap(v1, v2);
                return v1 == fou::static_vector<int, 3>{5, 6, 7} &&
                       v2 == fou::static_vector<int, 3>{0, 1, 2};
            }();
            STATIC_REQUIRE(result);
        }
        {
            constexpr auto result = [] {
                fou::static_vector<int, 5> v1{0, 1, 2, 3};
                fou::static_vector<int, 5> v2{5, 6, 7};
                swap(v1, v2);
                return v1 == fou::static_vector<int, 3>{5, 6, 7} &&
                       v2 == fou::static_vector<int, 4>{0, 1, 2, 3};
            }();
            STATIC_REQUIRE(result);
        }
        {
            constexpr auto result = [] {
                fou::static_vector<nontrivial_int, 5> v1{0, 1, 2};
                fou::static_vector<nontrivial_int, 5> v2{5, 6, 7};
                swap(v1, v2);
                return v1 == fou::static_vector<nontrivial_int, 3>{5, 6, 7} &&
                       v2 == fou::static_vector<nontrivial_int, 3>{0, 1, 2};
            }();
            STATIC_REQUIRE(result);
        }
        {
            fou::static_vector<nontrivial_int, 5> v1{0, 1, 2};
            fou::static_vector<nontrivial_int, 5> v2{5, 6, 7, 8};
            swap(v1, v2);
            CHECK(v1 == fou::static_vector<nontrivial_int, 4>{5, 6, 7, 8});
            CHECK(v2 == fou::static_vector<nontrivial_int, 3>{0, 1, 2});
        }
        {
            fou::static_vector<nontrivial_int, 5> v1{0, 1, 2, 3};
            fou::static_vector<nontrivial_int, 5> v2{5, 6, 7};
            swap(v1, v2);
            CHECK(v1 == fou::static_vector<nontrivial_int, 3>{5, 6, 7});
            CHECK(v2 == fou::static_vector<nontrivial_int, 4>{0, 1, 2, 3});
        }
        {
            fou::static_vector<nontrivial_int, 5> v{0, 1, 2};
            swap(v, v);
            CHECK(v == fou::static_vector<nontrivial_int, 3>{0, 1, 2});
        }
    }

    SECTION("span") {
        fou::static_vector<int, 5> v1{0, 1, 2};
        auto                       s1 = std::span{v1};
        CHECK(3 == s1.size());

        fou::static_vector<int, 5> const v2{3, 4, 5};
        auto                             s2 = std::span{v2};
        CHECK(3 == s2.size());
    }

    SECTION("end") {
        REQUIRE(nontrivial_counter::ctor_dtor_call_counter == 0);
    }
}