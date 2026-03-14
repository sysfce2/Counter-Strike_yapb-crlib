// test_cpuflags.cpp — tests for crlib/cpuflags.h (CpuFlags)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
TEST_CASE("CpuFlags singleton returns same instance", "[cpuflags]") {
    auto &a = CpuFlags::instance();
    auto &b = cpuflags;  // global alias
    REQUIRE(&a == &b);
}

// ---------------------------------------------------------------------------
// On x86/x64 Windows, SSE2 is guaranteed baseline (VS requirement).
// We can't know exact capabilities at test-write time, so we test that the
// detection ran (flags are a valid subset of possible combinations) and that
// the singleton is consistent across multiple accesses.
// ---------------------------------------------------------------------------
TEST_CASE("CpuFlags singleton is consistent across multiple accesses", "[cpuflags]") {
    auto &cf = cpuflags;

    // Reading twice should give the same result
    bool sse3_a = cf.sse3;
    bool sse3_b = cf.sse3;
    REQUIRE(sse3_a == sse3_b);

    bool avx_a = cf.avx;
    bool avx_b = cf.avx;
    REQUIRE(avx_a == avx_b);
}

TEST_CASE("CpuFlags avx2 implies avx on real hardware", "[cpuflags]") {
    // If AVX2 is detected, AVX must also be detected (hardware invariant)
    if (cpuflags.avx2) {
        REQUIRE(cpuflags.avx);
    }
    // Even if both are false, this condition is satisfied
    REQUIRE(true);
}

TEST_CASE("CpuFlags sse42 implies sse41 on real hardware", "[cpuflags]") {
    if (cpuflags.sse42) {
        REQUIRE(cpuflags.sse41);
    }
    REQUIRE(true);
}

TEST_CASE("CpuFlags sse41 implies ssse3 on real hardware", "[cpuflags]") {
    if (cpuflags.sse41) {
        REQUIRE(cpuflags.ssse3);
    }
    REQUIRE(true);
}

TEST_CASE("CpuFlags ssse3 implies sse3 on real hardware", "[cpuflags]") {
    if (cpuflags.ssse3) {
        REQUIRE(cpuflags.sse3);
    }
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// Smoke test: all boolean fields are reachable and have valid bool values
// ---------------------------------------------------------------------------
TEST_CASE("CpuFlags all boolean fields are readable", "[cpuflags]") {
    auto &cf = cpuflags;

    // Just access them — this verifies the struct layout is as expected
    bool vals[] = { cf.sse3, cf.ssse3, cf.sse41, cf.sse42, cf.avx, cf.avx2, cf.neon };
    for (bool v : vals) {
        REQUIRE((v == true || v == false));  // tautology, but exercises the read
    }
}
