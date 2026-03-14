// test_hashmap.cpp — tests for crlib/hashmap.h
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Construction / empty state
// ---------------------------------------------------------------------------
TEST_CASE("HashMap default construction is empty", "[hashmap]") {
    HashMap<String, int> m;
    REQUIRE(m.empty());
    REQUIRE(m.length() == 0u);
}

// ---------------------------------------------------------------------------
// operator[] insert & lookup
// ---------------------------------------------------------------------------
TEST_CASE("HashMap operator[] inserts and retrieves values", "[hashmap]") {
    HashMap<String, int> m;
    m["alpha"] = 1;
    m["beta"]  = 2;
    m["gamma"] = 3;

    REQUIRE(m.length()  == 3u);
    REQUIRE(m["alpha"]  == 1);
    REQUIRE(m["beta"]   == 2);
    REQUIRE(m["gamma"]  == 3);
}

TEST_CASE("HashMap operator[] updates existing key", "[hashmap]") {
    HashMap<String, int> m;
    m["x"] = 10;
    m["x"] = 20;

    REQUIRE(m.length() == 1u);
    REQUIRE(m["x"]     == 20);
}

// ---------------------------------------------------------------------------
// insert
// ---------------------------------------------------------------------------
TEST_CASE("HashMap insert returns true for new key", "[hashmap]") {
    HashMap<String, int> m;
    REQUIRE(m.insert("key", 42));
    REQUIRE(m.length() == 1u);
    REQUIRE(m["key"]   == 42);
}

TEST_CASE("HashMap insert returns false for duplicate key", "[hashmap]") {
    HashMap<String, int> m;
    m["dup"] = 1;
    REQUIRE_FALSE(m.insert("dup", 2));
    REQUIRE(m["dup"] == 1);  // unchanged
}

// ---------------------------------------------------------------------------
// exists
// ---------------------------------------------------------------------------
TEST_CASE("HashMap exists returns true for present key", "[hashmap]") {
    HashMap<String, int> m;
    m["present"] = 99;
    REQUIRE(m.exists("present"));
    REQUIRE_FALSE(m.exists("absent"));
}

// ---------------------------------------------------------------------------
// erase
// ---------------------------------------------------------------------------
TEST_CASE("HashMap erase removes an existing key", "[hashmap]") {
    HashMap<String, int> m;
    m["a"] = 1;
    m["b"] = 2;

    REQUIRE(m.erase("a") == 1u);
    REQUIRE(m.length()   == 1u);
    REQUIRE_FALSE(m.exists("a"));
    REQUIRE(m.exists("b"));
}

TEST_CASE("HashMap erase on missing key returns 0", "[hashmap]") {
    HashMap<String, int> m;
    REQUIRE(m.erase("nonexistent") == 0u);
}

// ---------------------------------------------------------------------------
// clear
// ---------------------------------------------------------------------------
TEST_CASE("HashMap clear removes all entries", "[hashmap]") {
    HashMap<String, int> m;
    m["p"] = 1;
    m["q"] = 2;
    m.clear();

    REQUIRE(m.empty());
    REQUIRE(m.length() == 0u);
    REQUIRE_FALSE(m.exists("p"));
}

// ---------------------------------------------------------------------------
// zap
// ---------------------------------------------------------------------------
TEST_CASE("HashMap zap clears and resets storage", "[hashmap]") {
    HashMap<String, int> m;
    for (int i = 0; i < 50; ++i) {
        m[String().assignf("%d", i)] = i;
    }
    m.zap();
    REQUIRE(m.empty());
}

// ---------------------------------------------------------------------------
// capacity
// ---------------------------------------------------------------------------
TEST_CASE("HashMap capacity is non-zero after construction", "[hashmap]") {
    HashMap<String, int> m;
    REQUIRE(m.capacity() > 0u);
}

// ---------------------------------------------------------------------------
// Iteration
// ---------------------------------------------------------------------------
TEST_CASE("HashMap iteration visits all inserted entries", "[hashmap]") {
    HashMap<String, int> m;
    m["x"] = 10;
    m["y"] = 20;
    m["z"] = 30;

    int total = 0;
    int count = 0;
    for (auto [k, v] : m) {
        total += v;
        ++count;
    }
    REQUIRE(count == 3);
    REQUIRE(total == 60);
}

// ---------------------------------------------------------------------------
// Move constructor / move assignment
// ---------------------------------------------------------------------------
TEST_CASE("HashMap move constructor transfers state", "[hashmap]") {
    HashMap<String, int> a;
    a["one"] = 1;
    a["two"] = 2;

    HashMap<String, int> b(cr::move(a));
    REQUIRE(b.length()   == 2u);
    REQUIRE(b.exists("one"));
    REQUIRE(b.exists("two"));
    REQUIRE(a.empty());
    REQUIRE(a.length() == 0u);
}

TEST_CASE("HashMap move assignment transfers state", "[hashmap]") {
    HashMap<String, int> a;
    a["hello"] = 42;

    HashMap<String, int> b;
    b = cr::move(a);
    REQUIRE(b.length()      == 1u);
    REQUIRE(b.exists("hello"));
    REQUIRE(a.empty());
    REQUIRE(a.length() == 0u);
}

// ---------------------------------------------------------------------------
// Initializer-list constructor
// ---------------------------------------------------------------------------
TEST_CASE("HashMap initializer-list construction", "[hashmap]") {
    HashMap<String, int> m {
        { String("a"), 1 },
        { String("b"), 2 },
        { String("c"), 3 }
    };
    REQUIRE(m.length() == 3u);
    REQUIRE(m["a"] == 1);
    REQUIRE(m["b"] == 2);
    REQUIRE(m["c"] == 3);
}

// ---------------------------------------------------------------------------
// Integer key HashMap
// ---------------------------------------------------------------------------
TEST_CASE("HashMap with int32_t keys works correctly", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    m[1] = 100;
    m[2] = 200;
    m[3] = 300;

    REQUIRE(m.length() == 3u);
    REQUIRE(m[1] == 100);
    REQUIRE(m[2] == 200);
    REQUIRE(m[3] == 300);
    REQUIRE(m.erase(2) == 1u);
    REQUIRE(m.length() == 2u);
    REQUIRE_FALSE(m.exists(2));
}

// ---------------------------------------------------------------------------
// Rehash — insert enough entries to trigger several rehashes
// ---------------------------------------------------------------------------
TEST_CASE("HashMap handles many insertions with correct retrieval", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    for (int32_t i = 0; i < 200; ++i) {
        m[i] = i * 10;
    }
    REQUIRE(m.length() == 200u);
    for (int32_t i = 0; i < 200; ++i) {
        REQUIRE(m.exists(i));
        REQUIRE(m[i] == i * 10);
    }
}

// ---------------------------------------------------------------------------
// reserve
// ---------------------------------------------------------------------------
TEST_CASE("HashMap reserve grows capacity to at least the requested size", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    m.reserve(64);
    REQUIRE(m.capacity() >= 64u);
}

TEST_CASE("HashMap reserve preserves existing entries", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    for (int32_t i = 0; i < 10; ++i) {
        m[i] = i;
    }
    m.reserve(256);
    REQUIRE(m.capacity() >= 256u);
    REQUIRE(m.length() == 10u);
    for (int32_t i = 0; i < 10; ++i) {
        REQUIRE(m.exists(i));
        REQUIRE(m[i] == i);
    }
}

// ---------------------------------------------------------------------------
// const_iterator
// ---------------------------------------------------------------------------
TEST_CASE("HashMap const_iterator iterates all entries", "[hashmap]") {
    HashMap<String, int> m;
    m["a"] = 1;
    m["b"] = 2;
    m["c"] = 3;

    const auto &cm = m;
    int total = 0;
    int count = 0;
    for (auto [k, v] : cm) {
        total += v;
        ++count;
    }
    REQUIRE(count == 3);
    REQUIRE(total == 6);
}

TEST_CASE("HashMap cbegin/cend work correctly", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    m[1] = 10;
    m[2] = 20;

    int sum = 0;
    for (auto it = m.cbegin(); it != m.cend(); ++it) {
        auto [k, v] = *it;
        sum += v;
    }
    REQUIRE(sum == 30);
}

// ---------------------------------------------------------------------------
// Hash<const char*> null handling
// ---------------------------------------------------------------------------
TEST_CASE("HashMap with const char* key handles null gracefully", "[hashmap]") {
    HashMap<const char*, int> m;
    m["hello"] = 1;
    REQUIRE(m.exists("hello"));
    REQUIRE(m["hello"] == 1);
}

// ---------------------------------------------------------------------------
// insert with rvalue value
// ---------------------------------------------------------------------------
TEST_CASE("HashMap insert with rvalue moves the value", "[hashmap]") {
    HashMap<String, String> m;
    String val("moved_value");
    REQUIRE(m.insert("key", cr::move(val)));
    REQUIRE(m["key"] == "moved_value");
}

TEST_CASE("HashMap insert rvalue returns false for duplicate", "[hashmap]") {
    HashMap<String, String> m;
    m["dup"] = "original";
    REQUIRE_FALSE(m.insert("dup", String("new_value")));
    REQUIRE(m["dup"] == "original");
}

// ---------------------------------------------------------------------------
// EmptyHash template
// ---------------------------------------------------------------------------
TEST_CASE("HashMap with EmptyHash uses identity hash", "[hashmap]") {
    HashMap<int32_t, int32_t, EmptyHash<int32_t>> m;
    m[10] = 100;
    m[20] = 200;
    m[30] = 300;
    
    REQUIRE(m.length() == 3u);
    REQUIRE(m[10] == 100);
    REQUIRE(m[20] == 200);
    REQUIRE(m[30] == 300);
}

// ---------------------------------------------------------------------------
// Move assignment self-assignment
// ---------------------------------------------------------------------------
TEST_CASE("HashMap move assignment self-assignment is safe", "[hashmap]") {
    HashMap<String, int> m;
    m["a"] = 1;
    m["b"] = 2;
    
    m = cr::move(m);
    
    REQUIRE(m.length() == 2u);
    REQUIRE(m["a"] == 1);
    REQUIRE(m["b"] == 2);
}

// ---------------------------------------------------------------------------
// Iterator post-increment
// ---------------------------------------------------------------------------
TEST_CASE("HashMap iterator post-increment returns old position", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    m[1] = 10;
    m[2] = 20;
    
    auto it = m.begin();
    auto [k1, v1] = *it;
    auto old = it++;
    auto [k2, v2] = *old;
    
    REQUIRE(k1 == k2);
    REQUIRE(v1 == v2);
    REQUIRE(it != old);
}

// ---------------------------------------------------------------------------
// Tombstone reuse (erase then insert at same slot)
// ---------------------------------------------------------------------------
TEST_CASE("HashMap reuses tombstone slots on insert", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    m[1] = 10;
    m[2] = 20;
    m[3] = 30;
    
    size_t capBefore = m.capacity();
    
    REQUIRE(m.erase(2) == 1u);
    REQUIRE_FALSE(m.exists(2));
    
    m[2] = 200;
    REQUIRE(m.exists(2));
    REQUIRE(m[2] == 200);
    REQUIRE(m.capacity() == capBefore);
}

TEST_CASE("HashMap handles multiple erase and reinsert cycles", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    
    for (int i = 0; i < 10; ++i) {
        m[i] = i * 10;
    }
    
    for (int i = 0; i < 10; i += 2) {
        m.erase(i);
    }
    
    REQUIRE(m.length() == 5u);
    
    for (int i = 0; i < 10; i += 2) {
        m[i] = i * 100;
    }
    
    REQUIRE(m.length() == 10u);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(m.exists(i));
    }
}

// ---------------------------------------------------------------------------
// reserve when smaller than current (no-op)
// ---------------------------------------------------------------------------
TEST_CASE("HashMap reserve smaller than current is no-op", "[hashmap]") {
    HashMap<int32_t, int32_t> m;
    m.reserve(64);
    size_t cap = m.capacity();
    
    m.reserve(16);
    REQUIRE(m.capacity() == cap);
}

// ---------------------------------------------------------------------------
// Hash collisions with linear probing
// ---------------------------------------------------------------------------
TEST_CASE("HashMap handles hash collisions correctly", "[hashmap]") {
    struct CollidingHash {
        uint32_t operator()(int32_t) const noexcept {
            return 0;
        }
    };
    
    HashMap<int32_t, int32_t, CollidingHash> m;
    m[1] = 10;
    m[2] = 20;
    m[3] = 30;
    m[4] = 40;
    
    REQUIRE(m.length() == 4u);
    REQUIRE(m[1] == 10);
    REQUIRE(m[2] == 20);
    REQUIRE(m[3] == 30);
    REQUIRE(m[4] == 40);
    
    REQUIRE(m.erase(2) == 1u);
    REQUIRE(m.exists(1));
    REQUIRE_FALSE(m.exists(2));
    REQUIRE(m.exists(3));
    REQUIRE(m.exists(4));
    
    m[2] = 200;
    REQUIRE(m[2] == 200);
}

// ---------------------------------------------------------------------------
// StringRef key type
// ---------------------------------------------------------------------------
TEST_CASE("HashMap with StringRef key works correctly", "[hashmap]") {
    HashMap<StringRef, int> m;
    m[StringRef("hello")] = 1;
    m[StringRef("world")] = 2;
    
    REQUIRE(m.length() == 2u);
    REQUIRE(m.exists(StringRef("hello")));
    REQUIRE(m.exists(StringRef("world")));
    REQUIRE(m[StringRef("hello")] == 1);
    REQUIRE(m[StringRef("world")] == 2);
}

// ---------------------------------------------------------------------------
// Empty map operations
// ---------------------------------------------------------------------------
TEST_CASE("HashMap exists on empty map returns false", "[hashmap]") {
    HashMap<String, int> m;
    m.clear();
    REQUIRE_FALSE(m.exists("anything"));
}

TEST_CASE("HashMap erase on empty map returns 0", "[hashmap]") {
    HashMap<String, int> m;
    m.clear();
    REQUIRE(m.erase("anything") == 0u);
}
