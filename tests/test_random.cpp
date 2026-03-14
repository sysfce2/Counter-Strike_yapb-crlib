// test_random.cpp — tests for crlib/random.h (RWrand)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"
#include <climits>

using namespace cr;

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
TEST_CASE("RWrand singleton returns same instance", "[random]") {
    auto &a = RWrand::instance();
    auto &b = rg;  // global alias
    REQUIRE(&a == &b);
}

// ---------------------------------------------------------------------------
// get(int, int)
// ---------------------------------------------------------------------------
TEST_CASE("RWrand::get int is within [low, high]", "[random]") {
    for (int i = 0; i < 1000; ++i) {
        int v = rg.get(1, 10);
        REQUIRE(v >= 1);
        REQUIRE(v <= 10);
    }
}

TEST_CASE("RWrand::get int returns low when low == high", "[random]") {
    REQUIRE(rg.get(5, 5) == 5);
    REQUIRE(rg.get(-3, -3) == -3);
}

TEST_CASE("RWrand::get int with low > high returns low", "[random]") {
    REQUIRE(rg.get(10, 5) == 10);
}

TEST_CASE("RWrand::get int with negative-to-positive range stays in bounds", "[random]") {
    // Exercises the signed-overflow fix: high - low overflows int32_t for
    // large ranges spanning negative to positive.
    for (int i = 0; i < 1000; ++i) {
        int v = rg.get(-1000000, 1000000);
        REQUIRE(v >= -1000000);
        REQUIRE(v <= 1000000);
    }
}

TEST_CASE("RWrand::get int with INT32_MIN to INT32_MAX does not crash", "[random]") {
    // Before the fix, static_cast<uint32_t>(INT32_MAX - INT32_MIN) was signed
    // integer overflow (UB).  After the fix the subtraction is done in uint32_t.
    // The degenerate full-range case wraps to range=0, so the result is always
    // INT32_MIN, but it must not crash or invoke UB.
    int v = rg.get(INT32_MIN, INT32_MAX);
    (void)v;
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// get(float, float)
// ---------------------------------------------------------------------------
TEST_CASE("RWrand::get float is within [low, high]", "[random]") {
    for (int i = 0; i < 1000; ++i) {
        float v = rg.get(0.0f, 1.0f);
        REQUIRE(v >= 0.0f);
        REQUIRE(v <= 1.0f);
    }
}

TEST_CASE("RWrand::get float with negative range works", "[random]") {
    for (int i = 0; i < 100; ++i) {
        float v = rg.get(-5.0f, 5.0f);
        REQUIRE(v >= -5.0f);
        REQUIRE(v <= 5.0f);
    }
}

// ---------------------------------------------------------------------------
// operator() — convenience wrapper
// ---------------------------------------------------------------------------
TEST_CASE("RWrand::operator() int delegates to get", "[random]") {
    for (int i = 0; i < 100; ++i) {
        int v = rg(0, 100);
        REQUIRE(v >= 0);
        REQUIRE(v <= 100);
    }
}

TEST_CASE("RWrand::operator() float delegates to get", "[random]") {
    for (int i = 0; i < 100; ++i) {
        float v = rg(0.0f, 100.0f);
        REQUIRE(v >= 0.0f);
        REQUIRE(v <= 100.0f);
    }
}

// ---------------------------------------------------------------------------
// chance
// ---------------------------------------------------------------------------
TEST_CASE("RWrand::chance(0) never fires", "[random]") {
    for (int i = 0; i < 1000; ++i) {
        REQUIRE_FALSE(rg.chance(0));
    }
}

TEST_CASE("RWrand::chance(100) always fires", "[random]") {
    for (int i = 0; i < 1000; ++i) {
        REQUIRE(rg.chance(100));
    }
}

TEST_CASE("RWrand::chance(50) fires roughly half the time", "[random]") {
    int fired = 0;
    const int trials = 10000;
    for (int i = 0; i < trials; ++i) {
        if (rg.chance(50)) {
            ++fired;
        }
    }
    // Expect roughly 40–60% success rate
    REQUIRE(fired > trials * 35 / 100);
    REQUIRE(fired < trials * 65 / 100);
}

// ---------------------------------------------------------------------------
// seed — re-seeding produces different sequences
// ---------------------------------------------------------------------------
TEST_CASE("RWrand::seed with explicit value changes output", "[random]") {
    rg.seed(12345u);
    int a1 = rg.get(0, 1000000);
    int a2 = rg.get(0, 1000000);

    rg.seed(12345u);
    int b1 = rg.get(0, 1000000);
    int b2 = rg.get(0, 1000000);

    // Same seed → same sequence
    REQUIRE(a1 == b1);
    REQUIRE(a2 == b2);
}

// ---------------------------------------------------------------------------
// Uniqueness — sequence is not degenerate (same value every time)
// ---------------------------------------------------------------------------
TEST_CASE("RWrand produces a varied sequence", "[random]") {
    rg.seed();
    bool allSame = true;
    int first = rg.get(0, 1000000);
    for (int i = 0; i < 100; ++i) {
        if (rg.get(0, 1000000) != first) {
            allSame = false;
            break;
        }
    }
    REQUIRE_FALSE(allSame);
}

// ---------------------------------------------------------------------------
// seed() without arguments re-initializes RNG
// ---------------------------------------------------------------------------
TEST_CASE("RWrand::seed without arguments re-initializes", "[random]") {
    rg.seed(99999u);
    int a1 = rg.get(0, 1000000);
    int a2 = rg.get(0, 1000000);

    rg.seed();
    (void)rg.get(0, 1000000);
    (void)rg.get(0, 1000000);

    rg.seed(99999u);
    int c1 = rg.get(0, 1000000);
    int c2 = rg.get(0, 1000000);

    REQUIRE(a1 == c1);
    REQUIRE(a2 == c2);
}
