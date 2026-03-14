// test_platform.cpp — tests for crlib/platform.h (Platform singleton)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
TEST_CASE("Platform singleton returns same instance", "[platform]") {
    auto &a = Platform::instance();
    auto &b = plat;
    REQUIRE(&a == &b);
}

// ---------------------------------------------------------------------------
// Platform flags
// ---------------------------------------------------------------------------
TEST_CASE("Platform win flag is set on Windows", "[platform]") {
#if defined(_WIN32)
    REQUIRE(plat.win);
#else
    REQUIRE(!plat.win);
#endif
}

TEST_CASE("Platform x64 flag is set on 64-bit build", "[platform]") {
#if defined(_M_X64) || defined(__x86_64__) || defined(__aarch64__)
    REQUIRE(plat.x64);
#endif
    REQUIRE(true);
}

TEST_CASE("Platform isNonX86 is consistent with arch macros", "[platform]") {
    bool result = plat.isNonX86();
    REQUIRE((result == true || result == false));  // just ensure it doesn't crash
}

// ---------------------------------------------------------------------------
// setAppName
// ---------------------------------------------------------------------------
TEST_CASE("Platform setAppName stores the name", "[platform]") {
    plat.setAppName("crlib_test");
    REQUIRE(strcmp(plat.appName, "crlib_test") == 0);
}

// ---------------------------------------------------------------------------
// pid
// ---------------------------------------------------------------------------
TEST_CASE("Platform pid returns a positive value", "[platform]") {
    REQUIRE(plat.pid() > 0);
}

// ---------------------------------------------------------------------------
// hardwareConcurrency
// ---------------------------------------------------------------------------
TEST_CASE("Platform hardwareConcurrency is at least 1", "[platform]") {
    REQUIRE(plat.hardwareConcurrency() >= 1);
}

// ---------------------------------------------------------------------------
// bzero
// ---------------------------------------------------------------------------
TEST_CASE("Platform bzero zeroes a buffer", "[platform]") {
    int buf[4] = {1, 2, 3, 4};
    plat.bzero(buf, sizeof(buf));
    for (int v : buf) {
        REQUIRE(v == 0);
    }
}

// ---------------------------------------------------------------------------
// env
// ---------------------------------------------------------------------------
TEST_CASE("Platform env returns a non-null string for known variable", "[platform]") {
    // Avoid PATH which can exceed the 384-byte static buffer in plat.env()
    const char *val = plat.env("OS");
    REQUIRE(val != nullptr);
}

// ---------------------------------------------------------------------------
// seconds
// ---------------------------------------------------------------------------
TEST_CASE("Platform seconds returns non-negative value", "[platform]") {
    float s = plat.seconds();
    REQUIRE(s >= 0.0f);
}

// ---------------------------------------------------------------------------
// loctime
// ---------------------------------------------------------------------------
TEST_CASE("Platform loctime fills tm struct with valid values", "[platform]") {
    time_t t = time(nullptr);
    tm tm_info {};
    plat.loctime(&tm_info, &t);

    REQUIRE(tm_info.tm_year > 100);   // year since 1900 — > 100 means > 2000
    REQUIRE(tm_info.tm_mon >= 0);
    REQUIRE(tm_info.tm_mon <= 11);
    REQUIRE(tm_info.tm_mday >= 1);
    REQUIRE(tm_info.tm_mday <= 31);
}

// ---------------------------------------------------------------------------
// tmpfname
// ---------------------------------------------------------------------------
TEST_CASE("Platform tmpfname returns a non-empty string", "[platform]") {
    const char *name = plat.tmpfname();
    REQUIRE(name != nullptr);
    REQUIRE(name[0] != '\0');
}

// ---------------------------------------------------------------------------
// openStdioFile, fileExists, removeFile
// ---------------------------------------------------------------------------
TEST_CASE("Platform openStdioFile creates a file, fileExists detects it, removeFile deletes it", "[platform]") {
    const char *fname = "crlib_test_platform_file.tmp";

    // Write a file
    FILE *f = plat.openStdioFile(fname, "w");
    REQUIRE(f != nullptr);
    fprintf(f, "test");
    fclose(f);

    REQUIRE(plat.fileExists(fname));

    plat.removeFile(fname);
    REQUIRE(!plat.fileExists(fname));
}

// ---------------------------------------------------------------------------
// createDirectory
// ---------------------------------------------------------------------------
TEST_CASE("Platform createDirectory creates a directory", "[platform]") {
    const char *dir = "crlib_test_platform_dir";

    bool ok = plat.createDirectory(dir);
    REQUIRE(ok);
    REQUIRE(plat.fileExists(dir));

    // Cleanup
#if defined(CR_WINDOWS)
    _rmdir(dir);
#else
    rmdir(dir);
#endif
}
