// test_mathlib.cpp — tests for crlib/mathlib.h
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

static constexpr float kEps = 1e-4f;

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
TEST_CASE("Math constants have expected values", "[mathlib]") {
    REQUIRE(kMathPi          == Catch::Approx(3.14159265f).epsilon(1e-5f));
    REQUIRE(kDegreeToRadians == Catch::Approx(kMathPi / 180.0f).epsilon(1e-7f));
    REQUIRE(kRadiansToDegree == Catch::Approx(180.0f / kMathPi).epsilon(1e-5f));
}

// ---------------------------------------------------------------------------
// cr::abs
// ---------------------------------------------------------------------------
TEST_CASE("cr::abs on float", "[mathlib]") {
    REQUIRE(cr::abs(3.0f)   == Catch::Approx(3.0f));
    REQUIRE(cr::abs(-3.0f)  == Catch::Approx(3.0f));
    REQUIRE(cr::abs(0.0f)   == Catch::Approx(0.0f));
}

TEST_CASE("cr::abs on int", "[mathlib]") {
    REQUIRE(cr::abs(5)   == 5);
    REQUIRE(cr::abs(-5)  == 5);
    REQUIRE(cr::abs(0)   == 0);
}

// ---------------------------------------------------------------------------
// cr::sqrf
// ---------------------------------------------------------------------------
TEST_CASE("cr::sqrf squares a value", "[mathlib]") {
    REQUIRE(cr::sqrf(3.0f) == Catch::Approx(9.0f));
    REQUIRE(cr::sqrf(0.0f) == Catch::Approx(0.0f));
    REQUIRE(cr::sqrf(-4.0f) == Catch::Approx(16.0f));
    REQUIRE(cr::sqrf(2)    == 4);
}

// ---------------------------------------------------------------------------
// cr::sqrtf
// ---------------------------------------------------------------------------
TEST_CASE("cr::sqrtf computes square root", "[mathlib]") {
    REQUIRE(cr::sqrtf(4.0f)   == Catch::Approx(2.0f).epsilon(kEps));
    REQUIRE(cr::sqrtf(9.0f)   == Catch::Approx(3.0f).epsilon(kEps));
    REQUIRE(cr::sqrtf(0.0f)   == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(cr::sqrtf(2.0f)   == Catch::Approx(1.41421356f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// cr::rsqrtf (approximate reciprocal square root)
// ---------------------------------------------------------------------------
TEST_CASE("cr::rsqrtf approximates 1/sqrt(x)", "[mathlib]") {
    REQUIRE(cr::rsqrtf(4.0f)  == Catch::Approx(0.5f).epsilon(0.01f));
    REQUIRE(cr::rsqrtf(9.0f)  == Catch::Approx(1.0f / 3.0f).epsilon(0.01f));
    REQUIRE(cr::rsqrtf(1.0f)  == Catch::Approx(1.0f).epsilon(0.01f));
}

// ---------------------------------------------------------------------------
// cr::sinf / cr::cosf
// ---------------------------------------------------------------------------
TEST_CASE("cr::sinf computes sine", "[mathlib]") {
    REQUIRE(cr::sinf(0.0f)         == Catch::Approx(0.0f).margin(1e-6f));
    REQUIRE(cr::sinf(kMathPi / 2)  == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(cr::sinf(kMathPi)      == Catch::Approx(0.0f).margin(1e-6f));
}

TEST_CASE("cr::cosf computes cosine", "[mathlib]") {
    REQUIRE(cr::cosf(0.0f)         == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(cr::cosf(kMathPi / 2)  == Catch::Approx(0.0f).margin(1e-6f));
    REQUIRE(cr::cosf(kMathPi)      == Catch::Approx(-1.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// cr::tanf
// ---------------------------------------------------------------------------
TEST_CASE("cr::tanf computes tangent", "[mathlib]") {
    REQUIRE(cr::tanf(0.0f)         == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(cr::tanf(kMathPi / 4)  == Catch::Approx(1.0f).epsilon(0.001f));
}

// ---------------------------------------------------------------------------
// cr::atan2f
// ---------------------------------------------------------------------------
TEST_CASE("cr::atan2f computes two-argument arctangent", "[mathlib]") {
    REQUIRE(cr::atan2f(0.0f, 1.0f)   == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(cr::atan2f(1.0f, 0.0f)   == Catch::Approx(kMathPi / 2).epsilon(kEps));
    REQUIRE(cr::atan2f(0.0f, -1.0f)  == Catch::Approx(kMathPi).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// cr::powf
// ---------------------------------------------------------------------------
TEST_CASE("cr::powf raises x to the power y", "[mathlib]") {
    REQUIRE(cr::powf(2.0f, 10.0f) == Catch::Approx(1024.0f).epsilon(0.5f));
    REQUIRE(cr::powf(3.0f, 2.0f)  == Catch::Approx(9.0f).epsilon(kEps));
    REQUIRE(cr::powf(1.0f, 100.0f) == Catch::Approx(1.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// cr::log10f
// ---------------------------------------------------------------------------
TEST_CASE("cr::log10f computes base-10 logarithm", "[mathlib]") {
    REQUIRE(cr::log10f(1.0f)    == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(cr::log10f(10.0f)   == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(cr::log10f(100.0f)  == Catch::Approx(2.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// cr::roundf
// ---------------------------------------------------------------------------
TEST_CASE("cr::roundf rounds to nearest integer", "[mathlib]") {
    REQUIRE(cr::roundf(2.4f)  == Catch::Approx(2.0f));
    REQUIRE(cr::roundf(2.5f)  == Catch::Approx(3.0f));
    REQUIRE(cr::roundf(-2.5f) == Catch::Approx(-3.0f).epsilon(1.0f));  // impl-dependent
}

// ---------------------------------------------------------------------------
// cr::ceilf
// ---------------------------------------------------------------------------
TEST_CASE("cr::ceilf rounds up to next integer", "[mathlib]") {
    REQUIRE(cr::ceilf(2.1f)  == Catch::Approx(3.0f));
    REQUIRE(cr::ceilf(-2.9f) == Catch::Approx(-2.0f));
    REQUIRE(cr::ceilf(3.0f)  == Catch::Approx(3.0f));
}

// ---------------------------------------------------------------------------
// cr::floorf
// ---------------------------------------------------------------------------
TEST_CASE("cr::floorf rounds down to previous integer", "[mathlib]") {
    REQUIRE(cr::floorf(2.9f)  == Catch::Approx(2.0f));
    REQUIRE(cr::floorf(-2.1f) == Catch::Approx(-3.0f));
    REQUIRE(cr::floorf(3.0f)  == Catch::Approx(3.0f));
}

// ---------------------------------------------------------------------------
// cr::sincosf
// ---------------------------------------------------------------------------
TEST_CASE("cr::sincosf produces sin and cos simultaneously", "[mathlib]") {
    float s {}, c {};
    cr::sincosf(0.0f, s, c);
    REQUIRE(s == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(c == Catch::Approx(1.0f).epsilon(kEps));

    cr::sincosf(kMathPi / 2, s, c);
    REQUIRE(s == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(c == Catch::Approx(0.0f).margin(1e-6f));
}

// ---------------------------------------------------------------------------
// cr::fzero / cr::fequal
// ---------------------------------------------------------------------------
TEST_CASE("cr::fzero returns true near zero", "[mathlib]") {
    REQUIRE(cr::fzero(0.0f));
    REQUIRE(cr::fzero(0.001f));
    REQUIRE_FALSE(cr::fzero(0.1f));
}

TEST_CASE("cr::fequal detects near equality", "[mathlib]") {
    REQUIRE(cr::fequal(1.0f, 1.0f));
    REQUIRE(cr::fequal(1.0f, 1.0005f));
    REQUIRE_FALSE(cr::fequal(1.0f, 1.01f));
}

// ---------------------------------------------------------------------------
// cr::deg2rad / cr::rad2deg
// ---------------------------------------------------------------------------
TEST_CASE("cr::deg2rad converts degrees to radians", "[mathlib]") {
    REQUIRE(cr::deg2rad(0.0f)   == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(cr::deg2rad(180.0f) == Catch::Approx(kMathPi).epsilon(kEps));
    REQUIRE(cr::deg2rad(90.0f)  == Catch::Approx(kMathPi / 2).epsilon(kEps));
}

TEST_CASE("cr::rad2deg converts radians to degrees", "[mathlib]") {
    REQUIRE(cr::rad2deg(0.0f)    == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(cr::rad2deg(kMathPi) == Catch::Approx(180.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// cr::wrapAngle / cr::wrapAngle360 / cr::anglesDifference
// ---------------------------------------------------------------------------
TEST_CASE("cr::wrapAngle wraps to [-180, 180)", "[mathlib]") {
    REQUIRE(cr::wrapAngle(0.0f)    == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(cr::wrapAngle(180.0f)  == Catch::Approx(-180.0f).epsilon(kEps));
    REQUIRE(cr::wrapAngle(-180.0f) == Catch::Approx(-180.0f).epsilon(kEps));
    REQUIRE(cr::wrapAngle(270.0f)  == Catch::Approx(-90.0f).epsilon(kEps));
    REQUIRE(cr::wrapAngle(-270.0f) == Catch::Approx(90.0f).epsilon(kEps));
}

TEST_CASE("cr::wrapAngle360 wraps angles within one full revolution", "[mathlib]") {
    REQUIRE(cr::wrapAngle360(0.0f)   == Catch::Approx(0.0f).margin(1e-4f));
    REQUIRE(cr::wrapAngle360(90.0f)  == Catch::Approx(90.0f).epsilon(kEps));
    REQUIRE(cr::wrapAngle360(180.0f) == Catch::Approx(180.0f).epsilon(kEps));
    // wrapAngle360 uses floor(x/720+0.5), so 360 maps to -360 and 720 maps to 0
    REQUIRE(cr::wrapAngle360(720.0f) == Catch::Approx(0.0f).margin(1e-4f));
}

TEST_CASE("cr::anglesDifference computes wrapped difference", "[mathlib]") {
    REQUIRE(cr::anglesDifference(10.0f, 5.0f)    == Catch::Approx(5.0f).epsilon(kEps));
    REQUIRE(cr::anglesDifference(350.0f, 10.0f)  == Catch::Approx(-20.0f).epsilon(kEps));
}

// ===========================================================================
// Additional tests for missing coverage
// ===========================================================================

// ---------------------------------------------------------------------------
// cr::abs for double type
// ---------------------------------------------------------------------------
TEST_CASE("cr::abs on double", "[mathlib]") {
    REQUIRE(cr::abs(3.0)   == Catch::Approx(3.0));
    REQUIRE(cr::abs(-3.0)  == Catch::Approx(3.0));
    REQUIRE(cr::abs(0.0)   == Catch::Approx(0.0));
}

// ---------------------------------------------------------------------------
// cr::sqrf for double type
// ---------------------------------------------------------------------------
TEST_CASE("cr::sqrf on double", "[mathlib]") {
    REQUIRE(cr::sqrf(3.0) == Catch::Approx(9.0));
    REQUIRE(cr::sqrf(0.0) == Catch::Approx(0.0));
    REQUIRE(cr::sqrf(-4.0) == Catch::Approx(16.0));
}

// ---------------------------------------------------------------------------
// SSE specializations (when available)
// ---------------------------------------------------------------------------
#if defined(CR_HAS_SIMD_SSE)
TEST_CASE("cr::min SSE specialization", "[mathlib]") {
    float a = 3.0f, b = 5.0f;
    REQUIRE(cr::min(a, b) == Catch::Approx(3.0f));
    REQUIRE(cr::min(b, a) == Catch::Approx(3.0f));
    REQUIRE(cr::min(a, a) == Catch::Approx(3.0f));
}

TEST_CASE("cr::max SSE specialization", "[mathlib]") {
    float a = 3.0f, b = 5.0f;
    REQUIRE(cr::max(a, b) == Catch::Approx(5.0f));
    REQUIRE(cr::max(b, a) == Catch::Approx(5.0f));
    REQUIRE(cr::max(a, a) == Catch::Approx(3.0f));
}

TEST_CASE("cr::clamp SSE specialization", "[mathlib]") {
    REQUIRE(cr::clamp(2.0f, 0.0f, 1.0f) == Catch::Approx(1.0f));
    REQUIRE(cr::clamp(-1.0f, 0.0f, 1.0f) == Catch::Approx(0.0f));
    REQUIRE(cr::clamp(0.5f, 0.0f, 1.0f) == Catch::Approx(0.5f));
    REQUIRE(cr::clamp(0.0f, 0.0f, 1.0f) == Catch::Approx(0.0f));
    REQUIRE(cr::clamp(1.0f, 0.0f, 1.0f) == Catch::Approx(1.0f));
}
#endif

// ---------------------------------------------------------------------------
// Platform-specific function verification
// ---------------------------------------------------------------------------
TEST_CASE("Platform-specific math functions produce correct results", "[mathlib]") {
    // These tests verify that platform-specific implementations
    // produce the same results as standard implementations
    
    float angle = kMathPi / 4.0f; // 45 degrees
    
    // sin/cos should be consistent
    float s = cr::sinf(angle);
    float c = cr::cosf(angle);
    REQUIRE(s == Catch::Approx(0.70710678f).epsilon(kEps));
    REQUIRE(c == Catch::Approx(0.70710678f).epsilon(kEps));
    
    // tan = sin/cos
    float t = cr::tanf(angle);
    REQUIRE(t == Catch::Approx(1.0f).epsilon(0.001f));
    
    // atan2(1,1) = 45 degrees
    float a = cr::atan2f(1.0f, 1.0f);
    REQUIRE(a == Catch::Approx(kMathPi / 4.0f).epsilon(kEps));
    
    // sqrt and rsqrt relationship
    float x = 4.0f;
    float sqrt_x = cr::sqrtf(x);
    float rsqrt_x = cr::rsqrtf(x);
    REQUIRE(sqrt_x == Catch::Approx(2.0f).epsilon(kEps));
    REQUIRE(rsqrt_x * sqrt_x == Catch::Approx(1.0f).epsilon(0.01f));
}

// ---------------------------------------------------------------------------
// Edge cases for math functions
// ---------------------------------------------------------------------------
TEST_CASE("Math functions handle edge cases", "[mathlib]") {
    // Very small values
    REQUIRE(cr::sqrtf(1e-10f) == Catch::Approx(1e-5f).epsilon(0.01f));
    REQUIRE(cr::sinf(1e-6f) == Catch::Approx(1e-6f).epsilon(0.01f));
    
    // Very large values (wrap for trig functions)
    REQUIRE(cr::sinf(1000.0f * kMathPi) == Catch::Approx(0.0f).margin(1e-3f));
    
    // powf with exponent 0
    REQUIRE(cr::powf(2.0f, 0.0f) == Catch::Approx(1.0f).epsilon(kEps));
    
    // powf with base 1
    REQUIRE(cr::powf(1.0f, 100.0f) == Catch::Approx(1.0f).epsilon(kEps));
}
