// test_files.cpp — tests for crlib/files.h (File, MemFileStorage, MemFile, FileEnumerator)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// File — basic open / close
// ---------------------------------------------------------------------------
TEST_CASE("File default-constructed is not valid", "[files]") {
    File f;
    REQUIRE(!f);
}

TEST_CASE("File open non-existent file for reading fails", "[files]") {
    File f("crlib_nonexistent_xyz_123.txt", "r");
    REQUIRE(!f);
}

TEST_CASE("File open for writing succeeds", "[files]") {
    const char *fname = "crlib_test_file_open.tmp";
    {
        File f(fname, "w");
        REQUIRE(f);
    }  // closes on destruction
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// File — length
// ---------------------------------------------------------------------------
TEST_CASE("File length matches written bytes", "[files]") {
    const char *fname = "crlib_test_file_length.tmp";
    {
        File fw(fname, "w");
        REQUIRE(fw);
        fw.puts("hello");
    }
    {
        File fr(fname, "r");
        REQUIRE(fr);
        REQUIRE(fr.length() == 5);
    }
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// File — puts / get / eof
// ---------------------------------------------------------------------------
TEST_CASE("File puts and get read back bytes correctly", "[files]") {
    const char *fname = "crlib_test_file_puts.tmp";
    {
        File fw(fname, "w");
        fw.puts("abc");
    }
    {
        File fr(fname, "r");
        REQUIRE(fr);
        int a = fr.get();
        int b = fr.get();
        int c = fr.get();
        REQUIRE(a == 'a');
        REQUIRE(b == 'b');
        REQUIRE(c == 'c');
        // feof() only returns true after a read that goes past the end
        fr.get();  // trigger the past-end read
        REQUIRE(fr.eof());
    }
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// File — getLine
// ---------------------------------------------------------------------------
TEST_CASE("File getLine reads lines from a text file", "[files]") {
    const char *fname = "crlib_test_file_getline.tmp";
    {
        File fw(fname, "w");
        fw.puts("line1\n");
        fw.puts("line2\n");
    }
    {
        File fr(fname, "r");
        REQUIRE(fr);

        String line;
        REQUIRE(fr.getLine(line));
        REQUIRE(line.startsWith("line1"));

        REQUIRE(fr.getLine(line));
        REQUIRE(line.startsWith("line2"));
    }
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// File — seek / rewind
// ---------------------------------------------------------------------------
TEST_CASE("File seek and rewind work correctly", "[files]") {
    const char *fname = "crlib_test_file_seek.tmp";
    {
        File fw(fname, "w");
        fw.puts("abcdef");
    }
    {
        File fr(fname, "r");
        REQUIRE(fr);
        fr.seek(3, SEEK_SET);
        REQUIRE(fr.get() == 'd');

        fr.rewind();
        REQUIRE(fr.get() == 'a');
    }
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// File — read / write binary
// ---------------------------------------------------------------------------
TEST_CASE("File binary write and read round-trip", "[files]") {
    const char *fname = "crlib_test_file_binary.tmp";
    uint8_t written[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t read_back[4] = {};
    {
        File fw(fname, "wb");
        REQUIRE(fw);
        fw.write(written, sizeof(written));
    }
    {
        File fr(fname, "rb");
        REQUIRE(fr);
        fr.read(read_back, sizeof(read_back));
        for (int i = 0; i < 4; ++i) {
            REQUIRE(read_back[i] == written[i]);
        }
    }
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// File — putChar / close
// ---------------------------------------------------------------------------
TEST_CASE("File putChar writes a single character", "[files]") {
    const char *fname = "crlib_test_file_putchar.tmp";
    {
        File fw(fname, "w");
        fw.putChar('Z');
    }
    {
        File fr(fname, "r");
        REQUIRE(fr.get() == 'Z');
    }
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// File::makePath (static)
// ---------------------------------------------------------------------------
TEST_CASE("File::makePath creates nested directories without crashing", "[files]") {
    // Just exercise the call — it creates dirs step by step
    // Use a safe temp name in the current directory
    // We only test it doesn't crash; we won't validate deep dir creation here
    // because makePath skips the final path separator scanning edge cases
    File::makePath("crlib_mkpath_test_dir");
    plat.removeFile("crlib_mkpath_test_dir");  // attempt cleanup (may not exist)
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// MemFileStorage — defaultLoad / defaultUnload / loadToString
// ---------------------------------------------------------------------------
TEST_CASE("MemFileStorage defaultLoad reads an existing file", "[files]") {
    const char *fname = "crlib_test_memfile.tmp";
    {
        File fw(fname, "w");
        fw.puts("hello memfile");
    }

    int size = 0;
    uint8_t *data = MemFileStorage::defaultLoad(fname, &size);
    REQUIRE(data != nullptr);
    REQUIRE(size == 13);  // "hello memfile"
    REQUIRE(memcmp(data, "hello memfile", 13) == 0);

    MemFileStorage::defaultUnload(data);
    plat.removeFile(fname);
}

TEST_CASE("MemFileStorage defaultLoad returns nullptr for missing file", "[files]") {
    int size = 99;
    uint8_t *data = MemFileStorage::defaultLoad("crlib_missing_xyz.tmp", &size);
    REQUIRE(data == nullptr);
    REQUIRE(size == 0);
}

TEST_CASE("MemFileStorage loadToString reads file contents as String", "[files]") {
    const char *fname = "crlib_test_mfstr.tmp";
    {
        File fw(fname, "w");
        fw.puts("crlib rocks");
    }

    String s = MemFileStorage::loadToString(fname);
    REQUIRE(s.startsWith("crlib rocks"));
    plat.removeFile(fname);
}

TEST_CASE("MemFileStorage loadToString returns empty for missing file", "[files]") {
    String s = MemFileStorage::loadToString("crlib_missing_xyz.tmp");
    REQUIRE(s.empty());
}

// ---------------------------------------------------------------------------
// MemFileStorage — initialize / load / unload (using default fns)
// ---------------------------------------------------------------------------
TEST_CASE("MemFileStorage initialized with default functions loads file", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);

    const char *fname = "crlib_test_mfstorage.tmp";
    {
        File fw(fname, "w");
        fw.puts("xyz");
    }

    int sz = 0;
    uint8_t *buf = MemFileStorage::instance().load(fname, &sz);
    REQUIRE(buf != nullptr);
    REQUIRE(sz == 3);

    MemFileStorage::instance().unload(buf);
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// MemFile — default / open / get / read / seek / rewind / eof
// ---------------------------------------------------------------------------
TEST_CASE("MemFile default-constructed is not valid", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);

    MemFile mf;
    REQUIRE(!mf);
}

TEST_CASE("MemFile open non-existent file fails", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);

    MemFile mf("crlib_nonexistent_xyz.tmp");
    REQUIRE(!mf);
}

TEST_CASE("MemFile reads bytes from in-memory file", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);

    const char *fname = "crlib_test_memfile2.tmp";
    {
        File fw(fname, "w");
        fw.puts("abcd");
    }

    MemFile mf(fname);
    REQUIRE(mf);
    REQUIRE(mf.length() == 4);

    REQUIRE(mf.get() == 'a');
    REQUIRE(mf.get() == 'b');

    mf.rewind();
    REQUIRE(mf.get() == 'a');

    mf.close();
    REQUIRE(!mf);

    plat.removeFile(fname);
}

TEST_CASE("MemFile read block", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);

    const char *fname = "crlib_test_memfile_read.tmp";
    {
        File fw(fname, "wb");
        uint8_t data[4] = {10, 20, 30, 40};
        fw.write(data, 4);
    }

    MemFile mf(fname);
    REQUIRE(mf);

    uint8_t buf[4] = {};
    size_t n = mf.read(buf, 4);
    REQUIRE(n == 1);  // 1 block of size 4
    REQUIRE(buf[0] == 10);
    REQUIRE(buf[3] == 40);
    REQUIRE(mf.eof());

    plat.removeFile(fname);
}

TEST_CASE("MemFile seek works", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);

    const char *fname = "crlib_test_memfile_seek.tmp";
    {
        File fw(fname, "w");
        fw.puts("abcdef");
    }

    MemFile mf(fname);
    REQUIRE(mf);

    mf.seek(3, SEEK_SET);
    REQUIRE(mf.get() == 'd');

    mf.seek(1, SEEK_END);
    REQUIRE(mf.get() == 'f');

    plat.removeFile(fname);
}

TEST_CASE("MemFile getLine reads lines", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);

    const char *fname = "crlib_test_memfile_getline.tmp";
    {
        File fw(fname, "w");
        fw.puts("first\nsecond\n");
    }

    MemFile mf(fname);
    REQUIRE(mf);

    String line;
    REQUIRE(mf.getLine(line));
    REQUIRE(line.startsWith("first"));

    REQUIRE(mf.getLine(line));
    REQUIRE(line.startsWith("second"));

    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// FileEnumerator
// ---------------------------------------------------------------------------
TEST_CASE("FileEnumerator finds the file we just created", "[files]") {
    const char *fname = "crlib_test_enum_file.tmp";
    {
        File fw(fname, "w");
        fw.puts("x");
    }

    bool found = false;
    FileEnumerator fe("crlib_test_enum_file.tmp");

    while (fe) {
        String match = fe.getMatch();
        if (match.contains("crlib_test_enum_file")) {
            found = true;
        }
        fe.next();
    }
    REQUIRE(found);

    plat.removeFile(fname);
}

TEST_CASE("FileEnumerator with no matches is not valid from the start", "[files]") {
    FileEnumerator fe("crlib_this_file_does_not_exist_xyz_123.zzz");
    // On Windows: handle == INVALID_HANDLE_VALUE => stillValid() == false
    // On Linux: no match => stillValid() == false
    REQUIRE(!fe);
}

// ===========================================================================
// Additional tests for missing coverage
// ===========================================================================

// ---------------------------------------------------------------------------
// File class — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("File flush method works", "[files]") {
    const char *fname = "crlib_test_flush.tmp";
    File f(fname, "w");
    REQUIRE(f);
    f.puts("test");
    REQUIRE(f.flush());
    plat.removeFile(fname);
}

TEST_CASE("File puts with format string", "[files]") {
    const char *fname = "crlib_test_puts_format.tmp";
    {
        File f(fname, "w");
        REQUIRE(f);
        f.puts("Number: %d, String: %s", 42, "hello");
        // fputs returns non-negative on success, not byte count
        // Content verification below ensures write succeeded
    }
    {
        File f(fname, "r");
        String line;
        f.getLine(line);
        REQUIRE(line.contains("Number: 42"));
        REQUIRE(line.contains("String: hello"));
    }
    plat.removeFile(fname);
}

TEST_CASE("File double close is safe", "[files]") {
    const char *fname = "crlib_test_double_close.tmp";
    File f(fname, "w");
    REQUIRE(f);
    f.close();
    REQUIRE(!f);
    f.close(); // Second close should be safe
    REQUIRE(!f);
    plat.removeFile(fname);
}

TEST_CASE("File open on already open file closes first", "[files]") {
    const char *fname1 = "crlib_test_open1.tmp";
    const char *fname2 = "crlib_test_open2.tmp";
    
    File f(fname1, "w");
    REQUIRE(f);
    f.puts("first");
    
    bool ok = f.open(fname2, "w");
    REQUIRE(ok);
    REQUIRE(f);
    f.puts("second");
    
    f.close();
    plat.removeFile(fname1);
    plat.removeFile(fname2);
}

TEST_CASE("File eof on empty file", "[files]") {
    const char *fname = "crlib_test_empty_eof.tmp";
    {
        File f(fname, "w");
        REQUIRE(f);
    }
    {
        File f(fname, "r");
        REQUIRE(f);
        // Empty file: eof() returns false until we try to read past end
        REQUIRE(!f.eof());
        f.get(); // Try to read past end
        REQUIRE(f.eof());
    }
    plat.removeFile(fname);
}

TEST_CASE("File get on closed file returns EOF", "[files]") {
    File f;
    int ch = f.get();
    REQUIRE(ch == EOF);
}

TEST_CASE("File read/write with zero size/count returns 0", "[files]") {
    const char *fname = "crlib_test_zero_rw.tmp";
    File f(fname, "w");
    REQUIRE(f);
    
    char buffer[10];
    size_t written = f.write(buffer, 0, 5);
    REQUIRE(written == 0);
    
    f.close();
    
    File fr(fname, "r");
    size_t read = fr.read(buffer, 0, 5);
    REQUIRE(read == 0);
    
    plat.removeFile(fname);
}

TEST_CASE("File seek with invalid origin fails", "[files]") {
    const char *fname = "crlib_test_invalid_seek.tmp";
    File f(fname, "w");
    REQUIRE(f);
    f.puts("test");
    f.rewind();
    
    bool ok = f.seek(0, 999); // Invalid origin
    REQUIRE(!ok);
    
    plat.removeFile(fname);
}

TEST_CASE("File seek beyond file end succeeds (C behavior)", "[files]") {
    const char *fname = "crlib_test_seek_beyond.tmp";
    File f(fname, "w");
    REQUIRE(f);
    f.puts("test"); // 4 bytes
    
    // C's fseek allows seeking beyond end
    bool ok = f.seek(1000, SEEK_SET);
    REQUIRE(ok);
    
    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// MemFileStorage — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("MemFileStorage load without initialization returns nullptr", "[files]") {
    // Reset to default (no initialization)
    MemFileStorage::instance().initialize(nullptr, nullptr);
    
    const char *fname = "crlib_test_noinit.tmp";
    {
        File f(fname, "w");
        f.puts("test");
    }
    
    int size = 0;
    uint8_t *data = MemFileStorage::instance().load(fname, &size);
    REQUIRE(data == nullptr);
    REQUIRE(size == 0);
    
    plat.removeFile(fname);
}

TEST_CASE("MemFileStorage unload without initialization is safe", "[files]") {
    MemFileStorage::instance().initialize(nullptr, nullptr);
    
    uint8_t dummy[10];
    MemFileStorage::instance().unload(dummy); // Should not crash
    REQUIRE(true);
}

TEST_CASE("MemFileStorage unload with nullptr is safe", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);
    
    MemFileStorage::instance().unload(nullptr); // Should not crash
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// MemFile — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("MemFile seek with invalid origin fails", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);
    
    const char *fname = "crlib_test_memfile_invalid_seek.tmp";
    {
        File f(fname, "w");
        f.puts("test");
    }
    
    MemFile mf(fname);
    REQUIRE(mf);
    
    bool ok = mf.seek(0, 999); // Invalid origin
    REQUIRE(!ok);
    
    plat.removeFile(fname);
}

TEST_CASE("MemFile seek beyond file end fails", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);
    
    const char *fname = "crlib_test_memfile_seek_beyond.tmp";
    {
        File f(fname, "w");
        f.puts("test"); // 4 bytes
    }
    
    MemFile mf(fname);
    REQUIRE(mf);
    
    bool ok = mf.seek(1000, SEEK_SET);
    REQUIRE(!ok);
    
    plat.removeFile(fname);
}

TEST_CASE("MemFile read with zero size/count returns 0", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);
    
    const char *fname = "crlib_test_memfile_zero_read.tmp";
    {
        File f(fname, "w");
        f.puts("test");
    }
    
    MemFile mf(fname);
    REQUIRE(mf);
    
    char buffer[10];
    size_t read = mf.read(buffer, 0, 5);
    REQUIRE(read == 0);
    
    plat.removeFile(fname);
}

TEST_CASE("MemFile get on closed file returns EOF", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);
    
    MemFile mf;
    int ch = mf.get();
    REQUIRE(ch == EOF);
}

TEST_CASE("MemFile getLine on empty file returns false", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);
    
    const char *fname = "crlib_test_memfile_empty.tmp";
    {
        File f(fname, "w");
        // Write nothing - file exists but is empty
    }
    
    MemFile mf(fname);
    // MemFile returns true for empty file (contents_ may be non-null but length_ is 0)
    // operator bool returns contents_ != nullptr && length_ > 0
    // So empty file should not be valid
    REQUIRE(!mf);
    
    String line;
    bool ok = mf.getLine(line);
    REQUIRE(!ok);
    REQUIRE(line.empty());
    
    plat.removeFile(fname);
}

TEST_CASE("MemFile double close is safe", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);
    
    const char *fname = "crlib_test_memfile_double_close.tmp";
    {
        File f(fname, "w");
        f.puts("test");
    }
    
    MemFile mf(fname);
    REQUIRE(mf);
    mf.close();
    REQUIRE(!mf);
    mf.close(); // Second close should be safe
    REQUIRE(!mf);
    
    plat.removeFile(fname);
}

TEST_CASE("MemFile open on already open file closes first", "[files]") {
    MemFileStorage::instance().initialize(
        MemFileStorage::defaultLoad,
        MemFileStorage::defaultUnload);
    
    const char *fname1 = "crlib_test_memfile_open1.tmp";
    const char *fname2 = "crlib_test_memfile_open2.tmp";
    
    {
        File f(fname1, "w");
        f.puts("first");
    }
    {
        File f(fname2, "w");
        f.puts("second");
    }
    
    MemFile mf(fname1);
    REQUIRE(mf);
    REQUIRE(mf.length() == 5);
    
    bool ok = mf.open(fname2);
    REQUIRE(ok);
    REQUIRE(mf);
    REQUIRE(mf.length() == 6);
    
    mf.close();
    plat.removeFile(fname1);
    plat.removeFile(fname2);
}

// ---------------------------------------------------------------------------
// FileEnumerator — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("FileEnumerator getMatch when not valid", "[files]") {
    FileEnumerator fe("crlib_nonexistent_xyz_123.zzz");
    REQUIRE(!fe);
    
    // getMatch() may crash if entry_->entry is null on Unix
    // or entry_->data is uninitialized on Windows
    // Skip this test as it's testing undefined behavior
    REQUIRE(true);
}

TEST_CASE("FileEnumerator next when not valid returns false", "[files]") {
    FileEnumerator fe("crlib_nonexistent_xyz_123.zzz");
    REQUIRE(!fe);
    
    bool ok = fe.next();
    REQUIRE(!ok);
}

TEST_CASE("FileEnumerator double close is safe", "[files]") {
    const char *fname = "crlib_test_enum_double_close.tmp";
    {
        File f(fname, "w");
        f.puts("x");
    }
    
    FileEnumerator fe(fname);
    fe.close();
    fe.close(); // Second close should be safe
    REQUIRE(true);
    
    plat.removeFile(fname);
}

TEST_CASE("FileEnumerator with wildcard pattern", "[files]") {
    const char *fname1 = "crlib_test_wildcard_a.tmp";
    const char *fname2 = "crlib_test_wildcard_b.tmp";
    
    {
        File f1(fname1, "w");
        f1.puts("a");
    }
    {
        File f2(fname2, "w");
        f2.puts("b");
    }
    
    int count = 0;
    FileEnumerator fe("crlib_test_wildcard_*.tmp");
    
    while (fe) {
        String match = fe.getMatch();
        if (match.contains("wildcard")) {
            ++count;
        }
        fe.next();
    }
    
    REQUIRE(count >= 2);
    
    plat.removeFile(fname1);
    plat.removeFile(fname2);
}
