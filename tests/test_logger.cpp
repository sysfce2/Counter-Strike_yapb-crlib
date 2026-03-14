// test_logger.cpp — tests for crlib/logger.h (SimpleLogger)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Helper: get a unique temp filename for log output
// ---------------------------------------------------------------------------
static const char *logFileName() {
    return "crlib_test_logger.tmp";
}

static void cleanupLog() {
    plat.removeFile(logFileName());
}

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
TEST_CASE("SimpleLogger singleton returns same instance", "[logger]") {
    auto &a = SimpleLogger::instance();
    auto &b = logger;
    REQUIRE(&a == &b);
}

// ---------------------------------------------------------------------------
// initialize + disableLogWrite
// ---------------------------------------------------------------------------
TEST_CASE("SimpleLogger initialize does not crash", "[logger]") {
    logger.initialize(logFileName(), nullptr);
    REQUIRE(true);
    cleanupLog();
}

TEST_CASE("SimpleLogger disableLogWrite disables file output", "[logger]") {
    cleanupLog();
    logger.initialize(logFileName(), nullptr);
    logger.disableLogWrite(true);

    logger.message("This should not appear in the file");

    // File should NOT have been created because writing is disabled
    REQUIRE(!plat.fileExists(logFileName()));

    logger.disableLogWrite(false);
}

// ---------------------------------------------------------------------------
// message
// ---------------------------------------------------------------------------
TEST_CASE("SimpleLogger message writes to log file", "[logger]") {
    cleanupLog();
    logger.initialize(logFileName(), nullptr);
    logger.disableLogWrite(false);

    logger.message("test message %d", 42);

    // File should now exist
    REQUIRE(plat.fileExists(logFileName()));
    cleanupLog();
}

// ---------------------------------------------------------------------------
// error
// ---------------------------------------------------------------------------
TEST_CASE("SimpleLogger error writes to log file", "[logger]") {
    cleanupLog();
    logger.initialize(logFileName(), nullptr);
    logger.disableLogWrite(false);

    logger.error("test error %s", "hello");

    REQUIRE(plat.fileExists(logFileName()));
    cleanupLog();
}

// ---------------------------------------------------------------------------
// print function callback
// ---------------------------------------------------------------------------
TEST_CASE("SimpleLogger invokes the print function on message", "[logger]") {
    cleanupLog();

    bool called = false;
    String captured;

    logger.initialize(logFileName(), [&called, &captured](const char *msg) {
        called = true;
        captured = msg;
    });
    logger.disableLogWrite(false);

    logger.message("callback_test");

    REQUIRE(called);
    REQUIRE(captured.contains("callback_test"));
    cleanupLog();
}

TEST_CASE("SimpleLogger invokes the print function on error", "[logger]") {
    cleanupLog();

    bool called = false;
    logger.initialize(logFileName(), [&called](const char *) {
        called = true;
    });
    logger.disableLogWrite(false);

    logger.error("some error");
    REQUIRE(called);
    cleanupLog();
}

// ---------------------------------------------------------------------------
// message with nullptr print function (no crash)
// ---------------------------------------------------------------------------
TEST_CASE("SimpleLogger message with nullptr print function does not crash", "[logger]") {
    cleanupLog();
    logger.initialize(logFileName(), nullptr);
    logger.disableLogWrite(false);
    logger.message("no callback");
    REQUIRE(true);
    cleanupLog();
}

// ---------------------------------------------------------------------------
// LogFile — open and print (indirect through SimpleLogger)
// ---------------------------------------------------------------------------
TEST_CASE("SimpleLogger LogFile prints to a file directly", "[logger]") {
    const char *fname = "crlib_test_logfile.tmp";
    {
        SimpleLogger::LogFile lf(fname);
        lf.print("direct write");
    }
    REQUIRE(plat.fileExists(fname));

    // Verify content
    String content = MemFileStorage::loadToString(fname);
    REQUIRE(content.contains("direct write"));

    plat.removeFile(fname);
}
