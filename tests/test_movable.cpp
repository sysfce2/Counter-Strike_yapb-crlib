// test_movable.cpp — tests for crlib/movable.h (cr::move, cr::forward, cr::swap)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// cr::move
// ---------------------------------------------------------------------------
TEST_CASE("cr::move produces an rvalue reference", "[movable]") {
    int a = 42;
    int b = cr::move(a);
    REQUIRE(b == 42);
}

TEST_CASE("cr::move enables move construction", "[movable]") {
    struct Movable {
        int value { 0 };
        bool moved { false };
        Movable() = default;
        Movable(Movable &&rhs) noexcept : value(rhs.value), moved(true) {
            rhs.value = 0;
        }
    };

    Movable src;
    src.value = 99;

    Movable dst(cr::move(src));
    REQUIRE(dst.moved);
    REQUIRE(dst.value == 99);
    REQUIRE(src.value == 0);
}

// ---------------------------------------------------------------------------
// cr::forward
// ---------------------------------------------------------------------------
TEST_CASE("cr::forward passes lvalue as lvalue", "[movable]") {
    auto fwd = [](int &x) -> int & {
        return cr::forward<int &>(x);
    };
    int v = 7;
    int &ref = fwd(v);
    REQUIRE(&ref == &v);
}

TEST_CASE("cr::forward passes rvalue as rvalue (via perfect forwarding)", "[movable]") {
    // Verify that forwarding preserves the value category at compile time
    // (runtime check: the value arrives correctly)
    auto consume = [](int &&x) { return x * 2; };
    int result = consume(cr::forward<int>(10));
    REQUIRE(result == 20);
}

// ---------------------------------------------------------------------------
// cr::swap — scalar types
// ---------------------------------------------------------------------------
TEST_CASE("cr::swap exchanges two integer values", "[movable]") {
    int a = 10, b = 20;
    cr::swap(a, b);
    REQUIRE(a == 20);
    REQUIRE(b == 10);
}

TEST_CASE("cr::swap exchanges two float values", "[movable]") {
    float a = 1.5f, b = 2.5f;
    cr::swap(a, b);
    REQUIRE(a == Catch::Approx(2.5f));
    REQUIRE(b == Catch::Approx(1.5f));
}

TEST_CASE("cr::swap with same value is idempotent", "[movable]") {
    int a = 5;
    cr::swap(a, a);
    REQUIRE(a == 5);
}

// ---------------------------------------------------------------------------
// cr::swap — movable structs
// ---------------------------------------------------------------------------
TEST_CASE("cr::swap works with move-only types", "[movable]") {
    struct MO {
        int v;
        MO(int v) : v(v) {}
        MO(MO &&rhs) noexcept : v(rhs.v) { rhs.v = 0; }
        MO &operator=(MO &&rhs) noexcept { v = rhs.v; rhs.v = 0; return *this; }
    };

    MO a(100), b(200);
    cr::swap(a, b);
    REQUIRE(a.v == 200);
    REQUIRE(b.v == 100);
}

// ---------------------------------------------------------------------------
// cr::swap — fixed-size arrays
// ---------------------------------------------------------------------------
TEST_CASE("cr::swap exchanges C arrays element-wise", "[movable]") {
    int a[3] = { 1, 2, 3 };
    int b[3] = { 4, 5, 6 };
    cr::swap(a, b);

    REQUIRE(a[0] == 4); REQUIRE(a[1] == 5); REQUIRE(a[2] == 6);
    REQUIRE(b[0] == 1); REQUIRE(b[1] == 2); REQUIRE(b[2] == 3);
}

TEST_CASE("cr::swap on same array (no-op)", "[movable]") {
    int a[3] = { 7, 8, 9 };
    cr::swap(a, a);   // should be a no-op (pointer equality check)
    REQUIRE(a[0] == 7);
    REQUIRE(a[1] == 8);
    REQUIRE(a[2] == 9);
}
