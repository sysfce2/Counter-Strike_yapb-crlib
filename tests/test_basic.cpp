// test_basic.cpp — tests for crlib/basic.h
// Include crlib first so __PLACEMENT_NEW_INLINE is defined before Catch2 headers.
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

// ---------------------------------------------------------------------------
// cr::bufsize
// ---------------------------------------------------------------------------
TEST_CASE("bufsize returns array length minus 1", "[basic]") {
    char buf128[128];
    REQUIRE(cr::bufsize(buf128) == 127u);

    char buf1[1];
    REQUIRE(cr::bufsize(buf1) == 0u);

    char buf512[512];
    REQUIRE(cr::bufsize(buf512) == 511u);
}

// ---------------------------------------------------------------------------
// cr::bit
// ---------------------------------------------------------------------------
TEST_CASE("bit shifts 1 by n positions", "[basic]") {
    REQUIRE(cr::bit(0) == 1u);
    REQUIRE(cr::bit(1) == 2u);
    REQUIRE(cr::bit(4) == 16u);
    REQUIRE(cr::bit(8) == 256u);
    REQUIRE(cr::bit(10) == 1024u);
}

// ---------------------------------------------------------------------------
// cr::min
// ---------------------------------------------------------------------------
TEST_CASE("min returns the smaller value", "[basic]") {
    REQUIRE(cr::min(1, 2)   == 1);
    REQUIRE(cr::min(2, 1)   == 1);
    REQUIRE(cr::min(-5, 5)  == -5);
    REQUIRE(cr::min(7, 7)   == 7);
    REQUIRE(cr::min(1.5f, 2.5f) == 1.5f);
    REQUIRE(cr::min(0.0f, -1.0f) == -1.0f);
}

// ---------------------------------------------------------------------------
// cr::max
// ---------------------------------------------------------------------------
TEST_CASE("max returns the larger value", "[basic]") {
    REQUIRE(cr::max(1, 2)   == 2);
    REQUIRE(cr::max(2, 1)   == 2);
    REQUIRE(cr::max(-5, 5)  == 5);
    REQUIRE(cr::max(7, 7)   == 7);
    REQUIRE(cr::max(1.5f, 2.5f) == 2.5f);
    REQUIRE(cr::max(0.0f, -1.0f) == 0.0f);
}

// ---------------------------------------------------------------------------
// cr::clamp
// ---------------------------------------------------------------------------
TEST_CASE("clamp constrains value to [a, b]", "[basic]") {
    REQUIRE(cr::clamp(5,   0,  10) == 5);
    REQUIRE(cr::clamp(-1,  0,  10) == 0);
    REQUIRE(cr::clamp(15,  0,  10) == 10);
    REQUIRE(cr::clamp(0,   0,  10) == 0);
    REQUIRE(cr::clamp(10,  0,  10) == 10);

    REQUIRE(cr::clamp(0.5f, 0.0f, 1.0f) == 0.5f);
    REQUIRE(cr::clamp(-0.5f, 0.0f, 1.0f) == 0.0f);
    REQUIRE(cr::clamp(1.5f,  0.0f, 1.0f) == 1.0f);
}

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
namespace {
    struct MyTestSingleton : public cr::Singleton<MyTestSingleton> {
        int value { 42 };
        explicit MyTestSingleton() = default;
    };
}

TEST_CASE("Singleton::instance returns the same object every time", "[basic]") {
    auto &a = MyTestSingleton::instance();
    auto &b = MyTestSingleton::instance();
    REQUIRE(&a == &b);
    REQUIRE(a.value == 42);

    a.value = 99;
    REQUIRE(b.value == 99);  // same object
    a.value = 42;            // restore
}

// ---------------------------------------------------------------------------
// NonCopyable / NonMovable — compile-time properties verified at runtime
// ---------------------------------------------------------------------------
TEST_CASE("NonCopyable-derived class is default-constructible", "[basic]") {
    struct NC : public cr::NonCopyable {
        explicit NC() = default;
        int x { 7 };
    };
    NC obj;
    REQUIRE(obj.x == 7);
}

TEST_CASE("NonMovable-derived class is default-constructible", "[basic]") {
    struct NM : public cr::NonMovable {
        explicit NM() = default;
        int y { 13 };
    };
    NM obj;
    REQUIRE(obj.y == 13);
}

// ---------------------------------------------------------------------------
// cr::bit_ceil
// ---------------------------------------------------------------------------
TEST_CASE("bit_ceil returns next power of two", "[basic]") {
    REQUIRE(cr::bit_ceil(0) == 1u);
    REQUIRE(cr::bit_ceil(1) == 1u);
    REQUIRE(cr::bit_ceil(2) == 2u);
    REQUIRE(cr::bit_ceil(3) == 4u);
    REQUIRE(cr::bit_ceil(4) == 4u);
    REQUIRE(cr::bit_ceil(5) == 8u);
    REQUIRE(cr::bit_ceil(7) == 8u);
    REQUIRE(cr::bit_ceil(8) == 8u);
    REQUIRE(cr::bit_ceil(9) == 16u);
    REQUIRE(cr::bit_ceil(100) == 128u);
    REQUIRE(cr::bit_ceil(1000) == 1024u);
    REQUIRE(cr::bit_ceil(1024) == 1024u);
    REQUIRE(cr::bit_ceil(1025) == 2048u);
}
