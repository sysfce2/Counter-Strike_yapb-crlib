// test_wavehelper.cpp — tests for crlib/wavehelper.h
// wavehelper.h does not include any crlib headers itself, so we pull in basic.h
// first to ensure platform detection macros are set.
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

#include <string.h>

// ---------------------------------------------------------------------------
// WaveHelper<true>  — little-endian (default on x86/x64 Windows)
// ---------------------------------------------------------------------------
using WaveLE = WaveHelper<true>;
using WaveBE = WaveHelper<false>;

TEST_CASE("WaveHelper<LE>::read16 passes through value unchanged", "[wavehelper]") {
    WaveLE wh;
    REQUIRE(wh.read16<uint16_t>(0x1234u) == 0x1234u);
    REQUIRE(wh.read16<int16_t>(0x0001u)  == 1);
    REQUIRE(wh.read16<uint16_t>(0u) == 0u);
}

TEST_CASE("WaveHelper<LE>::read32 passes through value unchanged", "[wavehelper]") {
    WaveLE wh;
    REQUIRE(wh.read32<uint32_t>(0x12345678u) == 0x12345678u);
    REQUIRE(wh.read32<int32_t>(100u)         == 100);
}

TEST_CASE("WaveHelper<BE>::read16 byte-swaps the value", "[wavehelper]") {
    WaveBE wh;
    REQUIRE(wh.read16<uint16_t>(0x1234u) == 0x3412u);
    REQUIRE(wh.read16<uint16_t>(0x0100u) == 0x0001u);
    REQUIRE(wh.read16<uint16_t>(0u)      == 0u);
}

TEST_CASE("WaveHelper<BE>::read32 byte-swaps the value", "[wavehelper]") {
    WaveBE wh;
    REQUIRE(wh.read32<uint32_t>(0x12345678u) == 0x78563412u);
    REQUIRE(wh.read32<uint32_t>(0x01000000u) == 0x00000001u);
}

TEST_CASE("WaveHelper<LE>::isWave returns true for WAVE identifier", "[wavehelper]") {
    WaveLE wh;
    char wave[4] = { 'W', 'A', 'V', 'E' };
    REQUIRE(wh.isWave(wave));
}

TEST_CASE("WaveHelper<LE>::isWave returns false for non-WAVE identifier", "[wavehelper]") {
    WaveLE wh;
    char data[4] = { 'R', 'I', 'F', 'F' };
    REQUIRE_FALSE(wh.isWave(data));

    char zeros[4] = { 0, 0, 0, 0 };
    REQUIRE_FALSE(wh.isWave(zeros));
}

TEST_CASE("WaveHelper<BE>::isWave checks uint32 magic number", "[wavehelper]") {
    WaveBE wh;
    // BE isWave: *reinterpret_cast<uint32_t*>(format) == 0x57415645
    // On a little-endian machine the bytes must be stored in LE byte order
    // so that the uint32_t reinterpretation yields 0x57415645.
    // 0x57415645 in LE memory: { 0x45, 0x56, 0x41, 0x57 } = { 'E','V','A','W' }
    char wave[4];
    auto val = static_cast<uint32_t>(0x57415645);
    memcpy(wave, &val, 4);
    REQUIRE(wh.isWave(wave));
}

// ---------------------------------------------------------------------------
// Header struct sanity check
// ---------------------------------------------------------------------------
TEST_CASE("WaveHelper::Header has expected size (44 bytes)", "[wavehelper]") {
    // Standard PCM WAV header is 44 bytes
    REQUIRE(sizeof(WaveHelper<>::Header) == 44u);
}

TEST_CASE("WaveHelper<LE> reads a synthesised WAV header correctly", "[wavehelper]") {
    WaveLE wh;

    WaveHelper<>::Header hdr {};
    memcpy(hdr.riff, "RIFF", 4);
    memcpy(hdr.wave, "WAVE", 4);
    memcpy(hdr.fmt, "fmt ", 4);
    hdr.subchunk1Size  = 16u;
    hdr.audioFormat    = 1u;   // PCM
    hdr.numChannels    = 2u;   // stereo
    hdr.sampleRate     = 44100u;
    hdr.byteRate       = 176400u;
    hdr.blockAlign     = 4u;
    hdr.bitsPerSample  = 16u;
    memcpy(hdr.dataChunkId, "data", 4);
    hdr.dataChunkLength = 1024u;

    // On LE, read16/read32 are identity operations
    REQUIRE(wh.read16<uint16_t>(hdr.audioFormat)   == 1u);
    REQUIRE(wh.read16<uint16_t>(hdr.numChannels)   == 2u);
    REQUIRE(wh.read32<uint32_t>(hdr.sampleRate)    == 44100u);
    REQUIRE(wh.read32<uint32_t>(hdr.byteRate)      == 176400u);
    REQUIRE(wh.read16<uint16_t>(hdr.bitsPerSample) == 16u);
    REQUIRE(wh.isWave(hdr.wave));
}
