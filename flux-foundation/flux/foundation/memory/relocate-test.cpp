#include <flux/foundation.hpp>

#include <catch2/catch.hpp>
#include <cstring>

using namespace flux;

static_assert(meta::trivially_relocatable<int>);
static_assert(meta::trivially_relocatable<const int>);
static_assert(meta::trivially_relocatable<int*>);
static_assert(meta::trivially_relocatable<int (*)()>);

static_assert(meta::trivially_relocatable<int[]>);
static_assert(meta::trivially_relocatable<int const[]>);
static_assert(meta::trivially_relocatable<int[4]>);
static_assert(meta::trivially_relocatable<int const[4]>);

static_assert(not meta::trivially_relocatable<void>);
static_assert(not meta::trivially_relocatable<void const>);
static_assert(not meta::trivially_relocatable<int()>);

static_assert(meta::relocatable<int>);
static_assert(meta::relocatable<const int>);
static_assert(meta::relocatable<int*>);
static_assert(meta::relocatable<int (*)()>);

static_assert(not meta::relocatable<int[]>);
static_assert(not meta::relocatable<int const[]>);
static_assert(not meta::relocatable<int[4]>);
static_assert(not meta::relocatable<int const[4]>);

static_assert(not meta::relocatable<void>);
static_assert(not meta::relocatable<void const>);
static_assert(not meta::relocatable<int()>);

struct A {
    int ia;
};
static_assert(meta::trivially_copyable<A> and meta::trivially_relocatable<A>);

struct B : A {
    int  ib;
    char cb;
};
static_assert(meta::trivially_copyable<B> and meta::trivially_relocatable<B>);

struct C : B {
    short sc;
};
static_assert(meta::trivially_copyable<A> and meta::trivially_relocatable<C>);

TEST_CASE("fou::relocate_at", "[flux-memory/relocate.hpp]") {
    SECTION("trivially copyable/relocatable at run-time (std::memmove)") {
        C  c1 = {{{1}, 2, 3}, 4};
        B& b1 = c1;
        B  b2 = {{5}, 6, 7};
        // Before std::memmove
        CHECK(4 == c1.sc);

        ::std::memmove(&b1, &b2, sizeof(B));
        // After std::memmove
#if defined(_WIN64)
        CHECK(4 == c1.sc); // windows handles this correctly
#else
        CHECK(4 != c1.sc); // 64, or 0, or anything but 4
#endif
    }

    SECTION("trivially copyable/relocatable at run-time") {
        C  c1 = {{{1}, 2, 3}, 4};
        B& b1 = c1;
        B  b2 = {{5}, 6, 7};
        // Before relocate_at
        CHECK(4 == c1.sc);

        fou::relocate_at(&b1, &b2);
        // After relocate_at
        CHECK(4 == c1.sc);
    }
}