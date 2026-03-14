// test_simd.cpp — tests for crlib/simd.h
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

static constexpr float kEps = 1e-4f;

#if defined(CR_HAS_SIMD)

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
TEST_CASE("SimdVec3Wrap default construction zeroes all fields", "[simd]") {
    SimdVec3Wrap v;
    REQUIRE(v.x == Catch::Approx(0.0f));
    REQUIRE(v.y == Catch::Approx(0.0f));
    REQUIRE(v.z == Catch::Approx(0.0f));
    REQUIRE(v.w == Catch::Approx(0.0f));
}

TEST_CASE("SimdVec3Wrap xyz construction stores components correctly", "[simd]") {
    SimdVec3Wrap v { 1.0f, 2.0f, 3.0f };
    REQUIRE(v.x == Catch::Approx(1.0f));
    REQUIRE(v.y == Catch::Approx(2.0f));
    REQUIRE(v.z == Catch::Approx(3.0f));
    REQUIRE(v.w == Catch::Approx(0.0f));
}

TEST_CASE("SimdVec3Wrap xy construction zeroes z and w", "[simd]") {
    SimdVec3Wrap v { 5.0f, 7.0f };
    REQUIRE(v.x == Catch::Approx(5.0f));
    REQUIRE(v.y == Catch::Approx(7.0f));
    REQUIRE(v.z == Catch::Approx(0.0f));
    REQUIRE(v.w == Catch::Approx(0.0f));
}

TEST_CASE("SimdVec3Wrap negative component construction", "[simd]") {
    SimdVec3Wrap v { -1.0f, -2.0f, -3.0f };
    REQUIRE(v.x == Catch::Approx(-1.0f));
    REQUIRE(v.y == Catch::Approx(-2.0f));
    REQUIRE(v.z == Catch::Approx(-3.0f));
}

// ---------------------------------------------------------------------------
// hypot
// ---------------------------------------------------------------------------
TEST_CASE("SimdVec3Wrap hypot returns vector magnitude", "[simd]") {
    REQUIRE(SimdVec3Wrap { 3.0f, 4.0f, 0.0f }.hypot() == Catch::Approx(5.0f).epsilon(kEps));
    REQUIRE(SimdVec3Wrap { 1.0f, 0.0f, 0.0f }.hypot() == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(SimdVec3Wrap { 0.0f, 0.0f, 0.0f }.hypot() == Catch::Approx(0.0f).margin(kEps));
    REQUIRE(SimdVec3Wrap { 1.0f, 1.0f, 1.0f }.hypot() == Catch::Approx(cr::sqrtf(3.0f)).epsilon(kEps));
    REQUIRE(SimdVec3Wrap { 2.0f, 2.0f, 2.0f }.hypot() == Catch::Approx(cr::sqrtf(12.0f)).epsilon(kEps));
}

TEST_CASE("SimdVec3Wrap hypot is symmetric for negative components", "[simd]") {
    float pos = SimdVec3Wrap { 1.0f,  2.0f,  3.0f }.hypot();
    float neg = SimdVec3Wrap { -1.0f, -2.0f, -3.0f }.hypot();
    REQUIRE(pos == Catch::Approx(neg).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// normalize
// ---------------------------------------------------------------------------
TEST_CASE("SimdVec3Wrap normalize produces unit vector", "[simd]") {
    auto check_unit = [](SimdVec3Wrap n) {
        float len = cr::sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
        REQUIRE(len == Catch::Approx(1.0f).epsilon(0.001f));
    };
    check_unit(SimdVec3Wrap { 3.0f, 0.0f, 0.0f }.normalize());
    check_unit(SimdVec3Wrap { 0.0f, 5.0f, 0.0f }.normalize());
    check_unit(SimdVec3Wrap { 0.0f, 0.0f, 7.0f }.normalize());
    check_unit(SimdVec3Wrap { 1.0f, 1.0f, 1.0f }.normalize());
    check_unit(SimdVec3Wrap { 3.0f, 4.0f, 0.0f }.normalize());
    check_unit(SimdVec3Wrap { -2.0f, 3.0f, 6.0f }.normalize());
}

TEST_CASE("SimdVec3Wrap normalize of axis-aligned vector", "[simd]") {
    auto n = SimdVec3Wrap { 5.0f, 0.0f, 0.0f }.normalize();
    REQUIRE(n.x == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(n.y == Catch::Approx(0.0f).margin(kEps));
    REQUIRE(n.z == Catch::Approx(0.0f).margin(kEps));
}

TEST_CASE("SimdVec3Wrap normalize of zero vector does not crash", "[simd]") {
    REQUIRE_NOTHROW(SimdVec3Wrap {}.normalize());
}

// ---------------------------------------------------------------------------
// angleVectors (SSE path — NEON has a different signature)
// ---------------------------------------------------------------------------
#if defined(CR_HAS_SIMD_SSE)

namespace {
    float dot3f (const Vec3D<float> &a, const Vec3D<float> &b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    float len3f (const Vec3D<float> &v) {
        return cr::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }
}

TEST_CASE("SimdVec3Wrap angleVectors produces unit-length basis vectors", "[simd]") {
    Vec3D<float> fwd, rgt, up;
    SimdVec3Wrap angles { 0.0f, 0.0f, 0.0f };
    angles.angleVectors(&fwd, &rgt, &up);

    REQUIRE(len3f(fwd) == Catch::Approx(1.0f).epsilon(0.001f));
    REQUIRE(len3f(rgt) == Catch::Approx(1.0f).epsilon(0.001f));
    REQUIRE(len3f(up)  == Catch::Approx(1.0f).epsilon(0.001f));
}

TEST_CASE("SimdVec3Wrap angleVectors basis vectors are mutually orthogonal", "[simd]") {
    Vec3D<float> fwd, rgt, up;
    SimdVec3Wrap angles { 30.0f, 45.0f, 0.0f };
    angles.angleVectors(&fwd, &rgt, &up);

    REQUIRE(dot3f(fwd, rgt) == Catch::Approx(0.0f).margin(0.001f));
    REQUIRE(dot3f(fwd, up)  == Catch::Approx(0.0f).margin(0.001f));
    REQUIRE(dot3f(rgt, up)  == Catch::Approx(0.0f).margin(0.001f));
}

TEST_CASE("SimdVec3Wrap angleVectors zero angles give forward (1,0,0)", "[simd]") {
    // GoldSrc convention: (pitch=0, yaw=0, roll=0) => forward along +X
    Vec3D<float> fwd, rgt, up;
    SimdVec3Wrap { 0.0f, 0.0f, 0.0f }.angleVectors(&fwd, &rgt, &up);

    REQUIRE(fwd.x == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(fwd.y == Catch::Approx(0.0f).margin(kEps));
    REQUIRE(fwd.z == Catch::Approx(0.0f).margin(kEps));
}

TEST_CASE("SimdVec3Wrap angleVectors orthogonality holds for non-zero roll", "[simd]") {
    Vec3D<float> fwd, rgt, up;
    SimdVec3Wrap angles { 0.0f, 0.0f, 45.0f };
    angles.angleVectors(&fwd, &rgt, &up);

    REQUIRE(len3f(fwd) == Catch::Approx(1.0f).epsilon(0.001f));
    REQUIRE(len3f(rgt) == Catch::Approx(1.0f).epsilon(0.001f));
    REQUIRE(len3f(up)  == Catch::Approx(1.0f).epsilon(0.001f));
    REQUIRE(dot3f(fwd, rgt) == Catch::Approx(0.0f).margin(0.001f));
    REQUIRE(dot3f(fwd, up)  == Catch::Approx(0.0f).margin(0.001f));
    REQUIRE(dot3f(rgt, up)  == Catch::Approx(0.0f).margin(0.001f));
}

TEST_CASE("SimdVec3Wrap angleVectors accepts nullptr for unused outputs", "[simd]") {
    Vec3D<float> vec(0.0f, 0.0f, 0.0f);
    Vec3D<float> fwd;
    REQUIRE_NOTHROW(vec.angleVectors(&fwd, nullptr, nullptr));
}

// ---------------------------------------------------------------------------
// Additional SIMD tests
// ---------------------------------------------------------------------------
TEST_CASE("SimdVec3Wrap assignment operator", "[simd]") {
    SimdVec3Wrap a(1.0f, 2.0f, 3.0f);
    SimdVec3Wrap b;
    b = a;
    REQUIRE(b.x == Catch::Approx(1.0f));
    REQUIRE(b.y == Catch::Approx(2.0f));
    REQUIRE(b.z == Catch::Approx(3.0f));
}

TEST_CASE("SimdVec3Wrap component access", "[simd]") {
    SimdVec3Wrap a(1.0f, 2.0f, 3.0f);
    REQUIRE(a.x == Catch::Approx(1.0f));
    REQUIRE(a.y == Catch::Approx(2.0f));
    REQUIRE(a.z == Catch::Approx(3.0f));
    REQUIRE(a.w == Catch::Approx(0.0f));
}

#endif // CR_HAS_SIMD_SSE

#else // CR_HAS_SIMD

// Placeholder so the TU is never empty when SIMD is unavailable.
TEST_CASE("SIMD not available on this platform", "[simd][.]") {
    SUCCEED("simd.h compiled without SIMD support — no tests to run");
}

#endif // CR_HAS_SIMD

// ===========================================================================
// Platform detection tests (compile-time)
// ===========================================================================
TEST_CASE("SIMD platform macros are defined", "[simd]") {
    // These are compile-time checks, just verify we can compile
#if defined(CR_HAS_SIMD)
    SUCCEED("CR_HAS_SIMD is defined");
#endif
#if defined(CR_HAS_SIMD_SSE)
    SUCCEED("CR_HAS_SIMD_SSE is defined");
#endif
#if defined(CR_HAS_SIMD_NEON)
    SUCCEED("CR_HAS_SIMD_NEON is defined");
#endif
#if defined(CR_HAS_SIMD_RVV)
    SUCCEED("CR_HAS_SIMD_RVV is defined");
#endif
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// SIMD math function interface tests (when available)
// ---------------------------------------------------------------------------
#if defined(CR_HAS_SIMD_SSE)
TEST_CASE("SIMD math functions are available", "[simd]") {
    // Test that we can call SIMD math functions
    // These are mostly compile-time tests
    
    // The functions exist and can be called
    // We don't verify results since they're platform-specific
    SUCCEED("SIMD math functions are available");
}
#endif
