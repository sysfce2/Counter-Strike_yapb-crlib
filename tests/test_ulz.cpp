// test_ulz.cpp — tests for crlib/ulz.h (ULZ compression)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

#include <string.h>

using namespace cr;

// Helper: compress then decompress, verify round-trip
static bool roundTrip(const uint8_t *src, int32_t srcLen, uint8_t *tmpBuf, uint8_t *outBuf, int32_t outBufLen) {
    auto &ulz = ULZ::instance();

    int32_t compLen = ulz.compress(const_cast<uint8_t *>(src), srcLen, tmpBuf);
    if (compLen <= 0) {
        return false;
    }
    int32_t decompLen = ulz.uncompress(tmpBuf, compLen, outBuf, outBufLen);
    if (decompLen != srcLen) {
        return false;
    }
    return memcmp(src, outBuf, srcLen) == 0;
}

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
TEST_CASE("ULZ singleton returns same instance", "[ulz]") {
    auto &a = ULZ::instance();
    auto &b = ULZ::instance();
    REQUIRE(&a == &b);
}

// ---------------------------------------------------------------------------
// Compress + Uncompress round-trip tests
// ---------------------------------------------------------------------------
TEST_CASE("ULZ compresses and decompresses a short repetitive buffer", "[ulz]") {
    // Highly compressible: 1024 'A' bytes
    const int32_t dataLen = 1024;
    uint8_t input[dataLen];
    memset(input, 'A', dataLen);

    // Compressed output buffer: ULZ::Excess extra bytes guaranteed
    uint8_t compressed[dataLen + ULZ::Excess];
    uint8_t restored[dataLen];

    REQUIRE(roundTrip(input, dataLen, compressed, restored, dataLen));
}

TEST_CASE("ULZ compresses and decompresses ASCII text", "[ulz]") {
    const char *text =
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. "
        "How vexingly quick daft zebras jump! ";
    int32_t dataLen = static_cast<int32_t>(strlen(text));

    uint8_t compressed[2048 + ULZ::Excess];
    uint8_t restored[2048];

    REQUIRE(roundTrip(reinterpret_cast<const uint8_t *>(text), dataLen,
                      compressed, restored, dataLen));
}

TEST_CASE("ULZ compresses and decompresses pseudo-random binary data", "[ulz]") {
    // Deterministic pseudo-random data (less compressible)
    const int32_t dataLen = 512;
    uint8_t input[dataLen];
    uint32_t state = 0xdeadbeef;
    for (int32_t i = 0; i < dataLen; ++i) {
        state = state * 1664525u + 1013904223u;
        input[i] = static_cast<uint8_t>(state >> 24);
    }

    uint8_t compressed[dataLen * 2 + ULZ::Excess];
    uint8_t restored[dataLen];

    REQUIRE(roundTrip(input, dataLen, compressed, restored, dataLen));
}

TEST_CASE("ULZ compresses repeated byte patterns efficiently", "[ulz]") {
    // Pattern: ABABABAB... (2-byte repeat)
    const int32_t dataLen = 2048;
    uint8_t input[dataLen];
    for (int32_t i = 0; i < dataLen; ++i) {
        input[i] = (i % 2 == 0) ? 'A' : 'B';
    }

    uint8_t compressed[dataLen + ULZ::Excess];
    uint8_t restored[dataLen];

    int32_t compLen = ULZ::instance().compress(input, dataLen, compressed);
    REQUIRE(compLen > 0);
    // Good compression expected for repeating patterns
    REQUIRE(compLen < dataLen);

    int32_t decompLen = ULZ::instance().uncompress(compressed, compLen, restored, dataLen);
    REQUIRE(decompLen == dataLen);
    REQUIRE(memcmp(input, restored, dataLen) == 0);
}

// ---------------------------------------------------------------------------
// Uncompress failure cases
// ---------------------------------------------------------------------------
TEST_CASE("ULZ uncompress returns failure for truncated input", "[ulz]") {
    const int32_t dataLen = 512;
    uint8_t input[dataLen];
    memset(input, 'X', dataLen);

    uint8_t compressed[dataLen + ULZ::Excess];
    uint8_t restored[dataLen];

    int32_t compLen = ULZ::instance().compress(input, dataLen, compressed);
    REQUIRE(compLen > 0);

    // Feed only half the compressed data → should fail
    if (compLen > 2) {
        int32_t result = ULZ::instance().uncompress(compressed, compLen / 2,
                                                    restored, dataLen);
        REQUIRE(result == ULZ::UncompressFailure);
    }
}

TEST_CASE("ULZ uncompress returns failure when output buffer is too small", "[ulz]") {
    const int32_t dataLen = 256;
    uint8_t input[dataLen];
    memset(input, 'Y', dataLen);

    uint8_t compressed[dataLen + ULZ::Excess];
    uint8_t tinyOutput[16];

    int32_t compLen = ULZ::instance().compress(input, dataLen, compressed);
    REQUIRE(compLen > 0);

    int32_t result = ULZ::instance().uncompress(compressed, compLen,
                                                tinyOutput, sizeof(tinyOutput));
    REQUIRE(result == ULZ::UncompressFailure);
}

// ---------------------------------------------------------------------------
// Large buffer round-trip
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Regression: missing distance-bytes bounds check (malformed input)
// ---------------------------------------------------------------------------
TEST_CASE("ULZ uncompress returns failure on truncated distance bytes", "[ulz]") {
    // A single match token (value 0x00 = 4-byte match, no literals) with no
    // distance bytes following.  Before the fix, load<uint16_t>(ip) read 2
    // bytes past the end of the compressed buffer (UB).  After the fix it
    // must return UncompressFailure.
    uint8_t malformed[] = { 0x00 };
    uint8_t out[16] {};

    int32_t result = ULZ::instance().uncompress(malformed, 1, out, sizeof(out));
    REQUIRE(result == ULZ::UncompressFailure);
}

// ---------------------------------------------------------------------------
// Regression: copy() overflow near end of output buffer
// ---------------------------------------------------------------------------
TEST_CASE("ULZ decompresses correctly into exact-size output buffer (no slack)", "[ulz]") {
    // Compress a run that produces at least one short (<8 byte) match or
    // literal at the very end of the output.  The decompressor's fast-path
    // copy() always writes at least 8 bytes; without the safe-copy fallback
    // it would write past restored[] when the remaining space < 8.
    // We intentionally size 'restored' to exactly dataLen with no excess.
    const int32_t dataLen = 64;
    uint8_t input[dataLen];
    // Pattern chosen so the last few bytes form a short match near the end.
    for (int32_t i = 0; i < dataLen; ++i) {
        input[i] = static_cast<uint8_t>((i < 32) ? 'A' : 'B');
    }

    uint8_t compressed[dataLen + ULZ::Excess];
    uint8_t restored[dataLen];   // no extra slack

    int32_t compLen = ULZ::instance().compress(input, dataLen, compressed);
    REQUIRE(compLen > 0);

    int32_t decompLen = ULZ::instance().uncompress(compressed, compLen, restored, dataLen);
    REQUIRE(decompLen == dataLen);
    REQUIRE(memcmp(input, restored, dataLen) == 0);
}

// ---------------------------------------------------------------------------
// Large buffer round-trip
// ---------------------------------------------------------------------------
TEST_CASE("ULZ handles a larger buffer (16 KB)", "[ulz]") {
    const int32_t dataLen = 16 * 1024;
    auto inputBuf    = makeUnique<uint8_t[]>(static_cast<size_t>(dataLen));
    auto compBuf     = makeUnique<uint8_t[]>(static_cast<size_t>(dataLen + ULZ::Excess));
    auto restoreBuf  = makeUnique<uint8_t[]>(static_cast<size_t>(dataLen));

    // Fill with a walking pattern
    for (int32_t i = 0; i < dataLen; ++i) {
        inputBuf[i] = static_cast<uint8_t>(i & 0xFF);
    }

    REQUIRE(roundTrip(inputBuf.get(), dataLen, compBuf.get(), restoreBuf.get(), dataLen));
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------
TEST_CASE("ULZ compress with zero-length input", "[ulz]") {
    uint8_t input[1] = {0};
    uint8_t compressed[ULZ::Excess];
    
    int32_t compLen = ULZ::instance().compress(input, 0, compressed);
    REQUIRE(compLen == 0);
}

TEST_CASE("ULZ compress with 1-byte input", "[ulz]") {
    uint8_t input[] = {'X'};
    uint8_t compressed[sizeof(input) + ULZ::Excess];
    uint8_t restored[sizeof(input)];
    
    REQUIRE(roundTrip(input, 1, compressed, restored, 1));
}

TEST_CASE("ULZ compress with 3-byte input (less than MinMatch)", "[ulz]") {
    uint8_t input[] = {'A', 'B', 'C'};
    uint8_t compressed[sizeof(input) + ULZ::Excess];
    uint8_t restored[sizeof(input)];
    
    REQUIRE(roundTrip(input, 3, compressed, restored, 3));
}

TEST_CASE("ULZ uncompress with zero-length input", "[ulz]") {
    uint8_t input[] = {0};
    uint8_t output[16];
    
    int32_t result = ULZ::instance().uncompress(input, 0, output, sizeof(output));
    REQUIRE(result == 0);
}

TEST_CASE("ULZ uncompress with invalid distance > current position", "[ulz]") {
    // Create a compressed buffer with distance larger than output position
    // Token: 0x00 (4-byte match, no literals)
    // Distance: 0x0001 (distance = 1, but output position is 0)
    uint8_t malformed[] = {0x00, 0x01, 0x00}; // token + distance bytes
    uint8_t out[16] = {0};
    
    int32_t result = ULZ::instance().uncompress(malformed, sizeof(malformed), out, sizeof(out));
    REQUIRE(result == ULZ::UncompressFailure);
}

TEST_CASE("ULZ uncompress with distance = 0 should fail", "[ulz]") {
    // Distance 0 is invalid (would cause infinite loop)
    uint8_t malformed[] = {0x00, 0x00, 0x00}; // token + distance = 0
    uint8_t out[16] = {0};
    
    int32_t result = ULZ::instance().uncompress(malformed, sizeof(malformed), out, sizeof(out));
    REQUIRE(result == ULZ::UncompressFailure);
}

TEST_CASE("ULZ uncompress with overlapping copy (dist < length)", "[ulz]") {
    // This tests the copy logic when source and destination overlap
    // We need to create valid compressed data that produces overlapping copies
    // Simple approach: compress data that has repeating patterns
    const char* pattern = "ABCDABCDABCDABCD";
    int32_t dataLen = static_cast<int32_t>(strlen(pattern));
    
    uint8_t compressed[256];
    uint8_t restored[256];
    
    int32_t compLen = ULZ::instance().compress(
        reinterpret_cast<const uint8_t*>(pattern), dataLen, compressed);
    REQUIRE(compLen > 0);
    
    int32_t decompLen = ULZ::instance().uncompress(
        compressed, compLen, restored, dataLen);
    REQUIRE(decompLen == dataLen);
    REQUIRE(memcmp(pattern, restored, dataLen) == 0);
}

TEST_CASE("ULZ compress with data at window size boundary", "[ulz]") {
    // Test with data size around window size (65536)
    const int32_t windowSize = 65536;
    const int32_t dataLen = windowSize + 100; // Slightly larger than window
    
    auto inputBuf = makeUnique<uint8_t[]>(static_cast<size_t>(dataLen));
    auto compBuf = makeUnique<uint8_t[]>(static_cast<size_t>(dataLen + ULZ::Excess));
    auto restoreBuf = makeUnique<uint8_t[]>(static_cast<size_t>(dataLen));
    
    // Fill with pattern that should compress well
    for (int32_t i = 0; i < dataLen; ++i) {
        inputBuf[i] = static_cast<uint8_t>((i / 100) & 0xFF); // Repeating pattern every 100 bytes
    }
    
    REQUIRE(roundTrip(inputBuf.get(), dataLen, compBuf.get(), restoreBuf.get(), dataLen));
}

TEST_CASE("ULZ constants have correct values", "[ulz]") {
    REQUIRE(ULZ::Excess == 16);
    REQUIRE(ULZ::UncompressFailure == -1);
}

TEST_CASE("ULZ compress returns positive length for compressible data", "[ulz]") {
    uint8_t input[100];
    memset(input, 'A', sizeof(input));
    uint8_t compressed[sizeof(input) + ULZ::Excess];
    
    int32_t compLen = ULZ::instance().compress(input, sizeof(input), compressed);
    REQUIRE(compLen > 0);
    REQUIRE(compLen < static_cast<int32_t>(sizeof(input))); // Should compress
}

TEST_CASE("ULZ uncompress with exact match at end of buffer", "[ulz]") {
    // Create data where a match ends exactly at buffer end
    uint8_t input[128];
    for (int32_t i = 0; i < 128; ++i) {
        input[i] = static_cast<uint8_t>(i & 0x7F); // Values 0-127
    }
    
    uint8_t compressed[256];
    uint8_t restored[128];
    
    int32_t compLen = ULZ::instance().compress(input, 128, compressed);
    REQUIRE(compLen > 0);
    
    int32_t decompLen = ULZ::instance().uncompress(compressed, compLen, restored, 128);
    REQUIRE(decompLen == 128);
    REQUIRE(memcmp(input, restored, 128) == 0);
}
