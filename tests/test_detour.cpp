// test_detour.cpp — tests for crlib/detour.h (Detour<T>)
//
// NOTE: We do NOT actually install any hooks because patching executable
// memory during unit tests is risky and platform-dependent. Instead we
// verify the default/initial state of the Detour object and the behaviour
// of the shim on unsupported platforms.
//
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// A simple function signature we can use as the template parameter
using VoidFn = void();
using IntFn  = int(int);

// ---------------------------------------------------------------------------
// Default construction
// ---------------------------------------------------------------------------
TEST_CASE("Detour default-constructed is not valid", "[detour]") {
    Detour<VoidFn> d;
    REQUIRE(!d.valid());
}

TEST_CASE("Detour default-constructed is not detoured", "[detour]") {
    Detour<VoidFn> d;
    REQUIRE(!d.detoured());
}

// ---------------------------------------------------------------------------
// detour() / restore() on invalid Detour return false
// ---------------------------------------------------------------------------
TEST_CASE("Detour detour() returns false when not initialized", "[detour]") {
    Detour<VoidFn> d;
    REQUIRE(!d.detour());
}

TEST_CASE("Detour restore() returns false when not initialized", "[detour]") {
    Detour<VoidFn> d;
    REQUIRE(!d.restore());
}

// ---------------------------------------------------------------------------
// install() with nullptr detour target — stays invalid until valid set
// ---------------------------------------------------------------------------
TEST_CASE("Detour remains invalid after install with null function", "[detour]") {
    Detour<VoidFn> d;
    d.install(nullptr, false);
    // original_ is still null, so valid() == false
    REQUIRE(!d.valid());
}

// ---------------------------------------------------------------------------
// On non-x86 / shim platforms: all methods return safe defaults
// (the shim Detour always has valid()==false, detoured()==false)
// ---------------------------------------------------------------------------
#if defined(CR_ARCH_ARM) || defined(CR_ARCH_PPC) || defined(CR_ARCH_RISCV) || defined(CR_PSVITA)
TEST_CASE("Detour shim initialize does not crash", "[detour]") {
    Detour<IntFn> d;
    d.initialize("", "", nullptr);
    REQUIRE(!d.valid());
    REQUIRE(!d.detoured());
}
#endif

// ---------------------------------------------------------------------------
// On x86 / x64: initialize with a real function pointer fills original_
// but valid() is still false until install() provides a detour_ pointer
// ---------------------------------------------------------------------------
#if !defined(CR_ARCH_NON_X86)
static int sampleFunction(int x) {
    return x + 1;
}

TEST_CASE("Detour initialize with real function does not crash", "[detour]") {
    Detour<IntFn> d;
    // On Windows: module="" falls back to using the passed address directly
    // On Linux: module/name are ignored, address is used directly
    d.initialize("", "", sampleFunction);
    // valid() requires both original_ AND detour_ to be non-null
    // We haven't called install() yet, so detour_ is still null
    REQUIRE(!d.valid());
}

static int replacementFunction(int x) {
    return x * 2;
}

TEST_CASE("Detour install sets valid() to true", "[detour]") {
    Detour<IntFn> d;
    d.initialize("", "", sampleFunction);
    d.install(reinterpret_cast<void*>(replacementFunction), false);
    REQUIRE(d.valid());
    // detoured() is still false because we passed enable=false to install()
    REQUIRE(!d.detoured());
}

TEST_CASE("Detour destructor restores safely", "[detour]") {
    {
        Detour<IntFn> d;
        d.initialize("", "", sampleFunction);
        d.install(reinterpret_cast<void*>(replacementFunction), false);
        REQUIRE(d.valid());
        // destructor calls restore() — should not crash even though hook is not patched
    }
    REQUIRE(true);
}

TEST_CASE("Detour detour() returns true when valid", "[detour]") {
    Detour<IntFn> d;
    d.initialize("", "", sampleFunction);
    d.install(reinterpret_cast<void*>(replacementFunction), false);
    REQUIRE(d.valid());
    REQUIRE(!d.detoured());
    REQUIRE(d.detour());
    REQUIRE(d.detoured());
}

TEST_CASE("Detour restore() returns true after detour", "[detour]") {
    Detour<IntFn> d;
    d.initialize("", "", sampleFunction);
    d.install(reinterpret_cast<void*>(replacementFunction), false);
    REQUIRE(d.detour());
    REQUIRE(d.detoured());
    REQUIRE(d.restore());
    REQUIRE(!d.detoured());
}

TEST_CASE("Detour multiple detour/restore cycles work", "[detour]") {
    Detour<IntFn> d;
    d.initialize("", "", sampleFunction);
    d.install(reinterpret_cast<void*>(replacementFunction), false);

    for (int i = 0; i < 3; ++i) {
        REQUIRE(!d.detoured());
        REQUIRE(d.detour());
        REQUIRE(d.detoured());
        REQUIRE(d.restore());
        REQUIRE(!d.detoured());
    }
}

TEST_CASE("Detour install with enable=true immediately detours", "[detour]") {
    Detour<IntFn> d;
    d.initialize("", "", sampleFunction);
    d.install(reinterpret_cast<void*>(replacementFunction), true);
    REQUIRE(d.valid());
    REQUIRE(d.detoured());
}

TEST_CASE("Detour double detour call is safe", "[detour]") {
    Detour<IntFn> d;
    d.initialize("", "", sampleFunction);
    d.install(reinterpret_cast<void*>(replacementFunction), false);
    REQUIRE(d.detour());
    REQUIRE(d.detoured());
    REQUIRE(d.detour());
    REQUIRE(d.detoured());
}

TEST_CASE("Detour double restore call is safe", "[detour]") {
    Detour<IntFn> d;
    d.initialize("", "", sampleFunction);
    d.install(reinterpret_cast<void*>(replacementFunction), false);
    REQUIRE(d.detour());
    REQUIRE(d.restore());
    REQUIRE(!d.detoured());
    REQUIRE(d.restore());
    REQUIRE(!d.detoured());
}

TEST_CASE("Detour constructor with parameters initializes correctly", "[detour]") {
    Detour<IntFn> d("", "", sampleFunction);
    d.install(reinterpret_cast<void*>(replacementFunction), false);
    REQUIRE(d.valid());
}
#endif
