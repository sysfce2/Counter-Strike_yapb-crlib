// test_string.cpp — tests for crlib/string.h (StringRef + String + Strings + Utf8Tools)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ===========================================================================
// StringRef
// ===========================================================================

TEST_CASE("StringRef default construction is empty", "[stringref]") {
    StringRef sr;
    REQUIRE(sr.empty());
    REQUIRE(sr.length() == 0u);
}

TEST_CASE("StringRef from C-string stores correct length", "[stringref]") {
    StringRef sr("hello");
    REQUIRE(sr.length() == 5u);
    REQUIRE_FALSE(sr.empty());
}

TEST_CASE("StringRef from C-string with explicit length", "[stringref]") {
    StringRef sr("hello", 3u);
    REQUIRE(sr.length() == 3u);
}

TEST_CASE("StringRef chars() returns the underlying pointer", "[stringref]") {
    StringRef sr("world");
    REQUIRE(sr.chars() != nullptr);
    REQUIRE(sr.chars()[0] == 'w');
}

TEST_CASE("StringRef operator[] accesses characters", "[stringref]") {
    StringRef sr("abc");
    REQUIRE(sr[0] == 'a');
    REQUIRE(sr[2] == 'c');
}

TEST_CASE("StringRef equality operators", "[stringref]") {
    StringRef a("foo");
    StringRef b("foo");
    StringRef c("bar");

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
    REQUIRE(a != c);
    REQUIRE_FALSE(a != b);
    REQUIRE(a == "foo");
    REQUIRE(a != "bar");
}

TEST_CASE("StringRef equals method", "[stringref]") {
    StringRef a("test");
    REQUIRE(a.equals("test"));
    REQUIRE_FALSE(a.equals("other"));
}

// ---------------------------------------------------------------------------
// hash
// ---------------------------------------------------------------------------
TEST_CASE("StringRef hash is consistent for same string", "[stringref]") {
    StringRef a("key");
    StringRef b("key");
    REQUIRE(a.hash() == b.hash());
}

TEST_CASE("StringRef hash differs for different strings", "[stringref]") {
    REQUIRE(StringRef("abc").hash() != StringRef("xyz").hash());
}

TEST_CASE("StringRef fnv1a32 static method produces known hash", "[stringref]") {
    // FNV-1a of empty string == basis 0x811c9dc5
    REQUIRE(StringRef::fnv1a32("") == 0x811c9dc5u);
}

// ---------------------------------------------------------------------------
// startsWith / endsWith / contains
// ---------------------------------------------------------------------------
TEST_CASE("StringRef startsWith", "[stringref]") {
    StringRef sr("hello world");
    REQUIRE(sr.startsWith("hello"));
    REQUIRE_FALSE(sr.startsWith("world"));
    REQUIRE(sr.startsWith(""));
}

TEST_CASE("StringRef endsWith", "[stringref]") {
    StringRef sr("hello world");
    REQUIRE(sr.endsWith("world"));
    REQUIRE_FALSE(sr.endsWith("hello"));
}

TEST_CASE("StringRef contains", "[stringref]") {
    StringRef sr("the quick brown fox");
    REQUIRE(sr.contains("quick"));
    REQUIRE(sr.contains("fox"));
    REQUIRE_FALSE(sr.contains("cat"));
}

// ---------------------------------------------------------------------------
// find / rfind
// ---------------------------------------------------------------------------
TEST_CASE("StringRef find by char", "[stringref]") {
    StringRef sr("abcabc");
    REQUIRE(sr.find('a') == 0u);
    REQUIRE(sr.find('c') == 2u);
    REQUIRE(sr.find('z') == StringRef::InvalidIndex);
    REQUIRE(sr.find('a', 1u) == 3u);
}

TEST_CASE("StringRef find by substring", "[stringref]") {
    StringRef sr("hello world hello");
    REQUIRE(sr.find("hello") == 0u);
    REQUIRE(sr.find("world") == 6u);
    REQUIRE(sr.find("xyz")   == StringRef::InvalidIndex);
}

TEST_CASE("StringRef rfind by char", "[stringref]") {
    StringRef sr("abcabc");
    REQUIRE(sr.rfind('c') == 5u);
    REQUIRE(sr.rfind('a') == 3u);
}

TEST_CASE("StringRef rfind finds char at index 0", "[stringref]") {
    // Regression: old loop started at length_ and stopped before i==0
    StringRef sr("xyz");
    REQUIRE(sr.rfind('x') == 0u);
    REQUIRE(sr.rfind('z') == 2u);
    REQUIRE(sr.rfind('q') == StringRef::InvalidIndex);
}

TEST_CASE("StringRef rfind by substring", "[stringref]") {
    StringRef sr("abcabc");
    REQUIRE(sr.rfind("abc") == 3u);
    REQUIRE(sr.rfind("xyz") == StringRef::InvalidIndex);
}

// ---------------------------------------------------------------------------
// findFirstOf / findLastOf / findFirstNotOf / findLastNotOf
// ---------------------------------------------------------------------------
TEST_CASE("StringRef findFirstOf", "[stringref]") {
    StringRef sr("hello");
    REQUIRE(sr.findFirstOf("aeiou") == 1u);  // 'e'
    REQUIRE(sr.findFirstOf("xyz")   == StringRef::InvalidIndex);
}

TEST_CASE("StringRef findLastOf", "[stringref]") {
    StringRef sr("hello");
    REQUIRE(sr.findLastOf("aeiou") == 4u);  // 'o'
}

TEST_CASE("StringRef findLastOf finds match at index 0", "[stringref]") {
    // Regression: old loop stopped before i==0
    StringRef sr("hello");
    REQUIRE(sr.findLastOf("h") == 0u);
    REQUIRE(sr.findLastOf("xyz") == StringRef::InvalidIndex);
}

TEST_CASE("StringRef findFirstNotOf", "[stringref]") {
    StringRef sr("aabbcc");
    REQUIRE(sr.findFirstNotOf("ab") == 4u);  // first 'c'
}

TEST_CASE("StringRef findLastNotOf", "[stringref]") {
    StringRef sr("aabbcc");
    REQUIRE(sr.findLastNotOf("c") == 3u);  // last 'b'
}

TEST_CASE("StringRef findLastNotOf finds result at index 0", "[stringref]") {
    // Regression: old loop stopped before i==0
    StringRef sr("xbbb");
    REQUIRE(sr.findLastNotOf("b") == 0u);
}

// ---------------------------------------------------------------------------
// countChar / countStr
// ---------------------------------------------------------------------------
TEST_CASE("StringRef countChar counts occurrences of character", "[stringref]") {
    StringRef sr("banana");
    REQUIRE(sr.countChar('a') == 3u);
    REQUIRE(sr.countChar('b') == 1u);
    REQUIRE(sr.countChar('z') == 0u);
}

TEST_CASE("StringRef countStr counts occurrences of substring", "[stringref]") {
    StringRef sr("abababab");
    REQUIRE(sr.countStr("ab") == 4u);
    REQUIRE(sr.countStr("xyz") == 0u);
}

// ---------------------------------------------------------------------------
// substr
// ---------------------------------------------------------------------------
TEST_CASE("StringRef substr extracts a sub-range", "[stringref]") {
    StringRef sr("hello world");
    StringRef sub = sr.substr(6u, 5u);
    REQUIRE(sub == "world");
    REQUIRE(sub.length() == 5u);
}

TEST_CASE("StringRef substr without count takes rest of string", "[stringref]") {
    StringRef sr("hello world");
    StringRef sub = sr.substr(6u);
    REQUIRE(sub == "world");
}

// ---------------------------------------------------------------------------
// split
// ---------------------------------------------------------------------------
TEST_CASE("StringRef split by delimiter", "[stringref]") {
    StringRef sr("a,b,c,d");
    // Use String as the split result type for reliable comparison
    auto parts = sr.split<String>(",");
    REQUIRE(parts.length() == 4u);
    REQUIRE(parts[0] == "a");
    REQUIRE(parts[1] == "b");
    REQUIRE(parts[2] == "c");
    REQUIRE(parts[3] == "d");
}

TEST_CASE("StringRef split by multi-char delimiter", "[stringref]") {
    // Regression: old code did ++pos (1 byte) instead of pos += delim.length()
    StringRef sr("a::b::c");
    auto parts = sr.split<String>("::");
    REQUIRE(parts.length() == 3u);
    REQUIRE(parts[0] == "a");
    REQUIRE(parts[1] == "b");
    REQUIRE(parts[2] == "c");
}

TEST_CASE("StringRef split with no match returns whole string", "[stringref]") {
    StringRef sr("hello");
    auto parts = sr.split<String>(",");
    REQUIRE(parts.length() == 1u);
    REQUIRE(parts[0] == "hello");
}

TEST_CASE("StringRef split by max length", "[stringref]") {
    StringRef sr("abcdef");
    auto parts = sr.split<String>(2u);
    REQUIRE(parts.length() == 3u);
    REQUIRE(parts[0] == "ab");
    REQUIRE(parts[1] == "cd");
    REQUIRE(parts[2] == "ef");
}

// ---------------------------------------------------------------------------
// as<float> / as<int>
// ---------------------------------------------------------------------------
TEST_CASE("StringRef as<int> converts numeric string", "[stringref]") {
    StringRef sr("123");
    REQUIRE(sr.as<int>() == 123);

    StringRef neg("-42");
    REQUIRE(neg.as<int>() == -42);
}

TEST_CASE("StringRef as<float> converts float string", "[stringref]") {
    StringRef sr("3.14");
    REQUIRE(sr.as<float>() == Catch::Approx(3.14f).epsilon(0.01f));
}

// ---------------------------------------------------------------------------
// begin / end
// ---------------------------------------------------------------------------
TEST_CASE("StringRef range-based for loop iterates characters", "[stringref]") {
    StringRef sr("abc");
    int count = 0;
    for (char c : sr) {
        (void)c;
        ++count;
    }
    REQUIRE(count == 3);
}

// ===========================================================================
// String
// ===========================================================================

TEST_CASE("String default construction is empty", "[string]") {
    String s;
    REQUIRE(s.empty());
    REQUIRE(s.length() == 0u);
}

TEST_CASE("String construction from C-string", "[string]") {
    String s("hello");
    REQUIRE(s.length() == 5u);
    REQUIRE(s == "hello");
}

TEST_CASE("String construction from char", "[string]") {
    String s('X');
    REQUIRE(s.length() == 1u);
    REQUIRE(s[0] == 'X');
}

TEST_CASE("String copy construction", "[string]") {
    String a("world");
    String b(a);
    REQUIRE(b == "world");
}

TEST_CASE("String move construction transfers ownership", "[string]") {
    String a("transfer");
    String b(cr::move(a));
    REQUIRE(b == "transfer");
    REQUIRE(a.empty());
}

// ---------------------------------------------------------------------------
// assign
// ---------------------------------------------------------------------------
TEST_CASE("String assign from C-string", "[string]") {
    String s;
    s.assign("assigned");
    REQUIRE(s == "assigned");
}

TEST_CASE("String assign from char", "[string]") {
    String s;
    s.assign('Z');
    REQUIRE(s.length() == 1u);
    REQUIRE(s[0] == 'Z');
}

// ---------------------------------------------------------------------------
// append / operator+=
// ---------------------------------------------------------------------------
TEST_CASE("String append builds string incrementally", "[string]") {
    String s("foo");
    s.append("bar");
    REQUIRE(s == "foobar");
    REQUIRE(s.length() == 6u);
}

TEST_CASE("String append char", "[string]") {
    String s("ab");
    s.append('c');
    REQUIRE(s == "abc");
}

TEST_CASE("String operator+= appends string", "[string]") {
    String s("hello");
    s += " world";
    REQUIRE(s == "hello world");
}

// ---------------------------------------------------------------------------
// assignf / appendf
// ---------------------------------------------------------------------------
TEST_CASE("String assignf formats a string", "[string]") {
    String s;
    s.assignf("value=%d", 42);
    REQUIRE(s == "value=42");
}

TEST_CASE("String appendf appends a formatted string", "[string]") {
    String s("x=");
    s.appendf("%d", 7);
    REQUIRE(s == "x=7");
}

// ---------------------------------------------------------------------------
// operator+
// ---------------------------------------------------------------------------
TEST_CASE("String operator+ concatenates strings", "[string]") {
    String a("hello");
    String b(" world");
    String c = a + b;
    REQUIRE(c == "hello world");

    String d = a + " again";
    REQUIRE(d == "hello again");
}

// ---------------------------------------------------------------------------
// at / operator[]
// ---------------------------------------------------------------------------
TEST_CASE("String at and operator[] access characters", "[string]") {
    String s("abcde");
    REQUIRE(s.at(0) == 'a');
    REQUIRE(s[4]    == 'e');

    s.at(0) = 'A';
    REQUIRE(s[0] == 'A');
}

// ---------------------------------------------------------------------------
// insert / erase
// ---------------------------------------------------------------------------
TEST_CASE("String insert inserts substring at index", "[string]") {
    String s("helo");
    REQUIRE(s.insert(3u, "l"));
    REQUIRE(s == "hello");
}

TEST_CASE("String insert past end appends", "[string]") {
    String s("hello");
    REQUIRE(s.insert(100u, " world"));
    REQUIRE(s == "hello world");
}

TEST_CASE("String erase removes characters", "[string]") {
    String s("hello world");
    REQUIRE(s.erase(5u, 6u));
    REQUIRE(s == "hello");
}

// ---------------------------------------------------------------------------
// replace
// ---------------------------------------------------------------------------
TEST_CASE("String replace substitutes all occurrences", "[string]") {
    String s("aababaa");
    size_t n = s.replace("a", "x");
    REQUIRE(n > 0u);
    REQUIRE(s == "xxbxbxx");
}

// ---------------------------------------------------------------------------
// lowercase / uppercase
// ---------------------------------------------------------------------------
TEST_CASE("String lowercase converts to lower case", "[string]") {
    String s("HELLO World 123");
    s.lowercase();
    REQUIRE(s == "hello world 123");
}

TEST_CASE("String uppercase converts to upper case", "[string]") {
    String s("hello world");
    s.uppercase();
    REQUIRE(s == "HELLO WORLD");
}

// ---------------------------------------------------------------------------
// ltrim / rtrim / trim
// ---------------------------------------------------------------------------
TEST_CASE("String ltrim strips leading whitespace", "[string]") {
    String s("  \t hello");
    s.ltrim();
    REQUIRE(s == "hello");
}

TEST_CASE("String rtrim strips trailing whitespace", "[string]") {
    String s("hello   \n");
    s.rtrim();
    REQUIRE(s == "hello");
}

TEST_CASE("String trim strips both ends", "[string]") {
    String s("  \t  hello world  \n");
    s.trim();
    REQUIRE(s == "hello world");
}

// ---------------------------------------------------------------------------
// contains / startsWith / endsWith / find / rfind
// ---------------------------------------------------------------------------
TEST_CASE("String contains delegates to StringRef", "[string]") {
    String s("the quick brown fox");
    REQUIRE(s.contains("quick"));
    REQUIRE_FALSE(s.contains("cat"));
}

TEST_CASE("String startsWith", "[string]") {
    String s("hello world");
    REQUIRE(s.startsWith("hello"));
    REQUIRE_FALSE(s.startsWith("world"));
}

TEST_CASE("String endsWith", "[string]") {
    String s("hello world");
    REQUIRE(s.endsWith("world"));
    REQUIRE_FALSE(s.endsWith("hello"));
}

TEST_CASE("String find by char", "[string]") {
    String s("abcabc");
    REQUIRE(s.find('a') == 0u);
    REQUIRE(s.find('z') == String::InvalidIndex);
}

TEST_CASE("String find by substring", "[string]") {
    String s("hello world");
    REQUIRE(s.find("world") == 6u);
    REQUIRE(s.find("xyz")   == String::InvalidIndex);
}

TEST_CASE("String rfind by char", "[string]") {
    String s("abcabc");
    REQUIRE(s.rfind('c') == 5u);
}

// ---------------------------------------------------------------------------
// countChar / countStr
// ---------------------------------------------------------------------------
TEST_CASE("String countChar counts occurrences", "[string]") {
    String s("banana");
    REQUIRE(s.countChar('a') == 3u);
}

TEST_CASE("String countStr counts substring occurrences", "[string]") {
    String s("ababab");
    REQUIRE(s.countStr("ab") == 3u);
}

// ---------------------------------------------------------------------------
// substr / split
// ---------------------------------------------------------------------------
TEST_CASE("String substr returns a new String", "[string]") {
    String s("hello world");
    String sub = s.substr(6u, 5u);
    REQUIRE(sub == "world");
}

TEST_CASE("String split by delimiter", "[string]") {
    String s("one,two,three");
    auto parts = s.split(",");
    REQUIRE(parts.length() == 3u);
    REQUIRE(parts[0] == "one");
    REQUIRE(parts[1] == "two");
    REQUIRE(parts[2] == "three");
}

// ---------------------------------------------------------------------------
// as<int> / as<float>
// ---------------------------------------------------------------------------
TEST_CASE("String as<int> converts numeric string", "[string]") {
    String s("789");
    REQUIRE(s.as<int>() == 789);
}

TEST_CASE("String as<float> converts float string", "[string]") {
    String s("2.5");
    REQUIRE(s.as<float>() == Catch::Approx(2.5f));
}

// ---------------------------------------------------------------------------
// hash
// ---------------------------------------------------------------------------
TEST_CASE("String hash is consistent for same content", "[string]") {
    String a("test");
    String b("test");
    REQUIRE(a.hash() == b.hash());
}

// ---------------------------------------------------------------------------
// clear / empty
// ---------------------------------------------------------------------------
TEST_CASE("String clear empties the string", "[string]") {
    String s("nonempty");
    s.clear();
    REQUIRE(s.empty());
}

// ---------------------------------------------------------------------------
// join
// ---------------------------------------------------------------------------
TEST_CASE("String::join concatenates array with delimiter", "[string]") {
    Array<String> parts;
    parts.push(String("a"));
    parts.push(String("b"));
    parts.push(String("c"));

    String joined = String::join(parts, ",");
    REQUIRE(joined == "a,b,c");
}

TEST_CASE("String::join with single element has no delimiter", "[string]") {
    Array<String> parts;
    parts.push(String("only"));

    String joined = String::join(parts, ",");
    REQUIRE(joined == "only");
}

TEST_CASE("String::join with empty array returns empty", "[string]") {
    Array<String> parts;
    String joined = String::join(parts, ",");
    REQUIRE(joined.empty());
}

// ---------------------------------------------------------------------------
// range-based for
// ---------------------------------------------------------------------------
TEST_CASE("String range-based for loop iterates characters", "[string]") {
    String s("hello");
    int count = 0;
    for (char c : s) {
        (void)c;
        ++count;
    }
    REQUIRE(count == 5);
}

// ---------------------------------------------------------------------------
// str() accessor
// ---------------------------------------------------------------------------
TEST_CASE("String str() returns a valid StringRef", "[string]") {
    String s("crlib");
    StringRef ref = s.str();
    REQUIRE(ref == "crlib");
    REQUIRE(ref.length() == 5u);
}

TEST_CASE("String str() on empty String returns non-null StringRef", "[string]") {
    // Regression: old str() returned { chars_.get(), 0 } which was nullptr for
    // a default-constructed String, making hash()/find()/contains() crash.
    String s;
    StringRef ref = s.str();
    REQUIRE(ref.chars() != nullptr);
    REQUIRE(ref.empty());
}

TEST_CASE("String hash on default-constructed String does not crash", "[string]") {
    String s;
    REQUIRE_NOTHROW(s.hash());
}

TEST_CASE("String contains on default-constructed String does not crash", "[string]") {
    String s;
    REQUIRE_NOTHROW(s.contains("x"));
    REQUIRE_FALSE(s.contains("x"));
}

// ===========================================================================
// Strings (pool)
// ===========================================================================

TEST_CASE("Strings::isEmpty detects null and empty C-strings", "[strings]") {
    REQUIRE(strings.isEmpty(nullptr));
    REQUIRE(strings.isEmpty(""));
    REQUIRE_FALSE(strings.isEmpty("x"));
}

TEST_CASE("Strings::matches performs case-insensitive comparison", "[strings]") {
    REQUIRE(strings.matches("Hello", "hello"));
    REQUIRE(strings.matches("ABC", "abc"));
    REQUIRE_FALSE(strings.matches("abc", "xyz"));
}

TEST_CASE("Strings::chars returns a writeable rotating buffer", "[strings]") {
    char *buf = strings.chars();
    REQUIRE(buf != nullptr);
}

TEST_CASE("Strings::copy copies a string safely", "[strings]") {
    char dst[16] {};
    strings.copy(dst, "hello", sizeof(dst));
    REQUIRE(strcmp(dst, "hello") == 0);
}

TEST_CASE("Strings::copy respects destination capacity", "[strings]") {
    char dst[4] {};
    strings.copy(dst, "hello", sizeof(dst));
    REQUIRE(dst[3] == '\0');  // always null-terminated
}

TEST_CASE("Strings::concat appends source to destination", "[strings]") {
    char dst[32] {};
    strings.copy(dst, "hello", sizeof(dst));
    strings.concat(dst, " world", sizeof(dst));
    REQUIRE(strcmp(dst, "hello world") == 0);
}

TEST_CASE("Strings::format formats into rotating buffer", "[strings]") {
    char *buf = strings.format("num=%d", 42);
    REQUIRE(buf != nullptr);
    REQUIRE(strcmp(buf, "num=42") == 0);
}

// ===========================================================================
// Additional edge case tests
// ===========================================================================

// ---------------------------------------------------------------------------
// StringRef nullptr handling
// ---------------------------------------------------------------------------
TEST_CASE("StringRef from nullptr is safe", "[stringref]") {
    StringRef sr(nullptr);
    REQUIRE(sr.empty());
    REQUIRE(sr.length() == 0u);
    REQUIRE(sr.chars() != nullptr);
}

TEST_CASE("StringRef from nullptr with length is safe", "[stringref]") {
    StringRef sr(nullptr, 10u);
    REQUIRE(sr.empty());
    REQUIRE(sr.length() == 0u);
}

TEST_CASE("StringRef equality with nullptr", "[stringref]") {
    StringRef sr("");
    REQUIRE(sr == nullptr);

    StringRef sr2("hello");
    REQUIRE_FALSE(sr2 == nullptr);
}

// ---------------------------------------------------------------------------
// String self-assignment edge cases
// ---------------------------------------------------------------------------
TEST_CASE("String assign from overlapping memory is safe", "[string]") {
    String s("hello world");
    s.assign(s.chars() + 6, 5);
    REQUIRE(s == "world");
}

TEST_CASE("String assign from its own substr is safe", "[string]") {
    String s("prefix_suffix");
    StringRef sub = s.str().substr(7);
    s.assign(sub.chars(), sub.length());
    REQUIRE(s == "suffix");
}

// ---------------------------------------------------------------------------
// find/rfind edge cases with memchr optimization
// ---------------------------------------------------------------------------
TEST_CASE("StringRef find empty pattern returns start", "[stringref]") {
    StringRef sr("hello");
    REQUIRE(sr.find("") == 0u);
    REQUIRE(sr.find("", 3u) == 3u);
}

TEST_CASE("StringRef rfind empty pattern returns length", "[stringref]") {
    StringRef sr("hello");
    REQUIRE(sr.rfind("") == 5u);
}

TEST_CASE("StringRef find with pattern longer than source fails", "[stringref]") {
    StringRef sr("hi");
    REQUIRE(sr.find("hello") == StringRef::InvalidIndex);
}

// ---------------------------------------------------------------------------
// String null-termination guarantees
// ---------------------------------------------------------------------------
TEST_CASE("String chars returns null-terminated string", "[string]") {
    String s("test");
    const char *c = s.chars();
    REQUIRE(c[4] == '\0');
}

TEST_CASE("String empty chars returns valid pointer", "[string]") {
    String s;
    REQUIRE(s.chars() != nullptr);
    REQUIRE(s.chars()[0] == '\0');
}

// ===========================================================================
// Additional tests for untested methods
// ===========================================================================

// ---------------------------------------------------------------------------
// String capacity methods
// ---------------------------------------------------------------------------
TEST_CASE("String capacity returns allocated buffer size", "[string]") {
    String s;
    REQUIRE(s.capacity() == 0u);
    s = "hello";
    REQUIRE(s.capacity() >= 5u);
}

TEST_CASE("String resize increases capacity when needed", "[string]") {
    String s("hello");
    size_t oldCapacity = s.capacity();
    // "hello" has length 5, capacity starts at max(16, 6) = 16
    // resize(10): needed = 5 + 10 + 1 = 16, capacity is already 16, so no change
    s.resize(10u);
    REQUIRE(s.capacity() == oldCapacity); // No change needed
    
    // Now resize by larger amount
    s.resize(100u); // needed = 5 + 100 + 1 = 106 > 16
    REQUIRE(s.capacity() > oldCapacity);
    REQUIRE(s == "hello"); // Content unchanged
}

// ---------------------------------------------------------------------------
// String constructors with length
// ---------------------------------------------------------------------------
TEST_CASE("String constructor with char* and length", "[string]") {
    const char *data = "hello world";
    String s(data, 5u);
    REQUIRE(s == "hello");
    REQUIRE(s.length() == 5u);
}

TEST_CASE("String constructor from StringRef", "[string]") {
    StringRef sr("test string");
    String s(sr);
    REQUIRE(s == "test string");
    REQUIRE(s.length() == 11u);
}

// ---------------------------------------------------------------------------
// String assignment with length
// ---------------------------------------------------------------------------
TEST_CASE("String assign with char* and length", "[string]") {
    String s("original");
    const char *data = "new content";
    s.assign(data, 3u);
    REQUIRE(s == "new");
    REQUIRE(s.length() == 3u);
}

TEST_CASE("String assign with String and length", "[string]") {
    String s("original");
    String other("another string");
    s.assign(other, 7u);
    REQUIRE(s == "another");
    REQUIRE(s.length() == 7u);
}

TEST_CASE("String append with char* and length", "[string]") {
    String s("prefix");
    const char *data = " suffix longer";
    s.append(data, 7u);
    REQUIRE(s == "prefix suffix");
    REQUIRE(s.length() == 13u);
}

TEST_CASE("String append with String and length", "[string]") {
    String s("start");
    String other(" and more text");
    s.append(other, 9u);
    REQUIRE(s == "start and more");
    REQUIRE(s.length() == 14u);
}

// ---------------------------------------------------------------------------
// String search methods
// ---------------------------------------------------------------------------
TEST_CASE("String findFirstOf finds any character from set", "[string]") {
    String s("hello world");
    REQUIRE(s.findFirstOf("aeiou") == 1u); // 'e' at position 1
    REQUIRE(s.findFirstOf("aeiou", 2u) == 4u); // 'o' at position 4
    REQUIRE(s.findFirstOf("xyz") == String::InvalidIndex);
}

TEST_CASE("String findLastOf finds last character from set", "[string]") {
    String s("hello world");
    REQUIRE(s.findLastOf("aeiou") == 7u); // 'o' at position 7
    REQUIRE(s.findLastOf("xyz") == String::InvalidIndex);
}

TEST_CASE("String findFirstNotOf finds first character not in set", "[string]") {
    String s("   hello");
    REQUIRE(s.findFirstNotOf(" ") == 3u); // 'h' at position 3
    // Starting at position 3, string is "hello"
    // Looking for first char not in "helo" - that's 'l' at position 5? Wait, 'l' is in "helo"
    // Actually "helo" contains h,e,l,o - so all chars in "hello" are in "helo"
    // Should return InvalidIndex
    REQUIRE(s.findFirstNotOf("helo", 3u) == String::InvalidIndex);
}

TEST_CASE("String findLastNotOf finds last character not in set", "[string]") {
    String s("hello   ");
    REQUIRE(s.findLastNotOf(" ") == 4u); // 'o' at position 4
}

// ---------------------------------------------------------------------------
// String split with maxLength
// ---------------------------------------------------------------------------
TEST_CASE("String split with maxLength splits by length", "[string]") {
    String s("abcdefghij");
    auto parts = s.split(3u); // split into chunks of max 3 chars
    REQUIRE(parts.length() == 4u); // "abc", "def", "ghi", "j"
    REQUIRE(parts[0] == "abc");
    REQUIRE(parts[1] == "def");
    REQUIRE(parts[2] == "ghi");
    REQUIRE(parts[3] == "j");
}

TEST_CASE("String split with maxLength=1 returns single character chunks", "[string]") {
    String s("abc");
    auto parts = s.split(1u);
    REQUIRE(parts.length() == 3u);
    REQUIRE(parts[0] == "a");
    REQUIRE(parts[1] == "b");
    REQUIRE(parts[2] == "c");
}

// ---------------------------------------------------------------------------
// String operators
// ---------------------------------------------------------------------------
TEST_CASE("String assignment from char", "[string]") {
    String s;
    s = 'A';
    REQUIRE(s == "A");
    REQUIRE(s.length() == 1u);
}

TEST_CASE("String operator+ with char lhs", "[string]") {
    String s("world");
    String result = 'h' + s;
    REQUIRE(result == "hworld");
}

TEST_CASE("String operator+ with const char* lhs", "[string]") {
    String s("world");
    String result = "hello " + s;
    REQUIRE(result == "hello world");
}

// ---------------------------------------------------------------------------
// Utf8Tools methods
// ---------------------------------------------------------------------------
TEST_CASE("Utf8Tools toUpper converts lowercase to uppercase", "[utf8tools]") {
    REQUIRE(utf8tools.toUpper('a') == 'A');
    REQUIRE(utf8tools.toUpper('z') == 'Z');
    REQUIRE(utf8tools.toUpper('A') == 'A');
    REQUIRE(utf8tools.toUpper('1') == '1');
}

TEST_CASE("Utf8Tools strToUpper converts string to uppercase", "[utf8tools]") {
    String s("Hello World 123");
    String upper = utf8tools.strToUpper(s);
    REQUIRE(upper == "HELLO WORLD 123");
}

TEST_CASE("Utf8Tools strToUpper handles empty string", "[utf8tools]") {
    String s;
    String upper = utf8tools.strToUpper(s);
    REQUIRE(upper.empty());
}

// ---------------------------------------------------------------------------
// Strings class methods
// ---------------------------------------------------------------------------
TEST_CASE("Strings joinPath combines paths", "[strings]") {
    String path = strings.joinPath("folder", "subfolder", "file.txt");
#if defined(CR_WINDOWS)
    REQUIRE(path == "folder\\subfolder\\file.txt");
#else
    REQUIRE(path == "folder/subfolder/file.txt");
#endif
}

TEST_CASE("Strings format with single argument", "[strings]") {
    char *result = strings.format("Number: %d", 42);
    REQUIRE(result != nullptr);
    REQUIRE(strcmp(result, "Number: 42") == 0);
}

// ---------------------------------------------------------------------------
// StringRef methods with start parameter
// ---------------------------------------------------------------------------
TEST_CASE("StringRef findFirstOf with start parameter", "[stringref]") {
    StringRef sr("hello world");
    REQUIRE(sr.findFirstOf("aeiou", 2u) == 4u); // 'o' at position 4
    REQUIRE(sr.findFirstOf("aeiou", 5u) == 7u); // 'o' at position 7
}

TEST_CASE("StringRef findFirstNotOf with start parameter", "[stringref]") {
    StringRef sr("   hello");
    REQUIRE(sr.findFirstNotOf(" ", 0u) == 3u);
    // Starting at position 3, string is "hello"
    // Looking for first char not in "hel" - that's 'o' at position 7
    REQUIRE(sr.findFirstNotOf("hel", 3u) == 7u); // 'o' at position 7
}
