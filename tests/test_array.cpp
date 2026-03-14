// test_array.cpp — tests for crlib/array.h
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
TEST_CASE("Array default construction is empty", "[array]") {
    Array<int> a;
    REQUIRE(a.empty());
    REQUIRE(a.length() == 0u);
    REQUIRE(a.capacity() == 0u);
}

TEST_CASE("Array constructed with default value", "[array]") {
    Array<int> a(5u, 0);
    REQUIRE(a.length() == 5u);
    for (size_t i = 0; i < 5; ++i) {
        REQUIRE(a[i] == 0);
    }
}

TEST_CASE("Array initializer-list construction", "[array]") {
    Array<int> a { 10, 20, 30 };
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == 10);
    REQUIRE(a[1] == 20);
    REQUIRE(a[2] == 30);
}

// ---------------------------------------------------------------------------
// push / emplace / pop / discard
// ---------------------------------------------------------------------------
TEST_CASE("Array push and pop round-trip", "[array]") {
    Array<int> a;
    REQUIRE(a.push(1));
    REQUIRE(a.push(2));
    REQUIRE(a.push(3));
    REQUIRE(a.length() == 3u);

    REQUIRE(a.pop() == 3);
    REQUIRE(a.length() == 2u);
    REQUIRE(a.pop() == 2);
    REQUIRE(a.pop() == 1);
    REQUIRE(a.empty());
}

TEST_CASE("Array emplace constructs in place", "[array]") {
    Array<int> a;
    a.emplace(42);
    a.emplace(100);
    REQUIRE(a.length() == 2u);
    REQUIRE(a[0] == 42);
    REQUIRE(a[1] == 100);
}

TEST_CASE("Array discard removes last element", "[array]") {
    Array<int> a { 1, 2, 3 };
    a.discard();
    REQUIRE(a.length() == 2u);
    REQUIRE(a.last() == 2);
}

// ---------------------------------------------------------------------------
// first / last / at / operator[]
// ---------------------------------------------------------------------------
TEST_CASE("Array first and last accessors", "[array]") {
    Array<int> a { 5, 10, 15 };
    REQUIRE(a.first() == 5);
    REQUIRE(a.last()  == 15);
}

TEST_CASE("Array at and operator[] access elements", "[array]") {
    Array<int> a { 7, 14, 21 };
    REQUIRE(a.at(0) == 7);
    REQUIRE(a.at(1) == 14);
    REQUIRE(a[2]    == 21);

    a.at(0) = 99;
    REQUIRE(a[0] == 99);
}

// ---------------------------------------------------------------------------
// insert
// ---------------------------------------------------------------------------
TEST_CASE("Array insert at index shifts existing elements", "[array]") {
    Array<int> a { 1, 3, 4 };
    int v = 2;
    REQUIRE(a.insert(1u, v));
    REQUIRE(a.length() == 4u);
    REQUIRE(a[0] == 1);
    REQUIRE(a[1] == 2);
    REQUIRE(a[2] == 3);
    REQUIRE(a[3] == 4);
}

TEST_CASE("Array insert beyond length appends", "[array]") {
    Array<int> a { 1, 2 };
    int v = 99;
    REQUIRE(a.insert(100u, v));
    REQUIRE(a.last() == 99);
}

// ---------------------------------------------------------------------------
// erase
// ---------------------------------------------------------------------------
TEST_CASE("Array erase removes elements and compacts", "[array]") {
    Array<int> a { 10, 20, 30, 40 };
    REQUIRE(a.erase(1u, 2u));      // remove elements at index 1 and 2
    REQUIRE(a.length() == 2u);
    REQUIRE(a[0] == 10);
    REQUIRE(a[1] == 40);
}

TEST_CASE("Array erase rejects out-of-bounds range", "[array]") {
    Array<int> a { 1, 2, 3 };
    // index + count > length_ must return false without corrupting length
    REQUIRE_FALSE(a.erase(2u, 2u));   // 2+2=4 > 3
    REQUIRE(a.length() == 3u);
    REQUIRE_FALSE(a.erase(3u, 1u));   // 3+1=4 > 3
    REQUIRE(a.length() == 3u);
}

TEST_CASE("Array shift removes first element", "[array]") {
    Array<int> a { 1, 2, 3 };
    REQUIRE(a.shift());
    REQUIRE(a.length() == 2u);
    REQUIRE(a[0] == 2);
}

TEST_CASE("Array unshift prepends element", "[array]") {
    Array<int> a { 2, 3 };
    REQUIRE(a.unshift(1));
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == 1);
    REQUIRE(a[1] == 2);
}

// ---------------------------------------------------------------------------
// remove
// ---------------------------------------------------------------------------
TEST_CASE("Array remove erases element by value", "[array]") {
    Array<int> a { 10, 20, 30 };
    REQUIRE(a.remove(a[1]));  // remove 20
    REQUIRE(a.length() == 2u);
    REQUIRE(a[0] == 10);
    REQUIRE(a[1] == 30);
}

// ---------------------------------------------------------------------------
// resize / reserve / ensure / shrink
// ---------------------------------------------------------------------------
TEST_CASE("Array resize grows with default-constructed elements", "[array]") {
    Array<int> a;
    REQUIRE(a.resize(4u));
    REQUIRE(a.length() == 4u);
    // Default int — whatever the default is, length is correct
}

TEST_CASE("Array resize shrinks by discarding tail", "[array]") {
    Array<int> a { 1, 2, 3, 4, 5 };
    REQUIRE(a.resize(3u));
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == 1);
    REQUIRE(a[2] == 3);
}

TEST_CASE("Array reserve pre-allocates capacity", "[array]") {
    Array<int> a;
    REQUIRE(a.reserve(100u));
    REQUIRE(a.capacity() >= 100u);
    REQUIRE(a.length() == 0u);  // reserve does not change length
}

TEST_CASE("Array ensure guarantees at-least length", "[array]") {
    Array<int> a { 1, 2 };
    REQUIRE(a.ensure(5u));
    REQUIRE(a.capacity() >= 5u);
}

TEST_CASE("Array shrink reduces capacity to length", "[array]") {
    Array<int> a;
    a.reserve(200u);
    a.push(1);
    a.push(2);
    REQUIRE(a.capacity() > 2u);

    REQUIRE(a.shrink());
    REQUIRE(a.capacity() == 2u);
    REQUIRE(a.length()   == 2u);
}

// ---------------------------------------------------------------------------
// fill / clear / empty
// ---------------------------------------------------------------------------
TEST_CASE("Array fill replaces all elements with given value", "[array]") {
    Array<int> a { 1, 2, 3 };
    a.fill(7);
    REQUIRE(a.length() == 3u);
    for (size_t i = 0; i < a.length(); ++i) {
        REQUIRE(a[i] == 7);
    }
}

TEST_CASE("Array clear empties the array", "[array]") {
    Array<int> a { 1, 2, 3 };
    a.clear();
    REQUIRE(a.empty());
    REQUIRE(a.length() == 0u);
}

// ---------------------------------------------------------------------------
// reverse
// ---------------------------------------------------------------------------
TEST_CASE("Array reverse flips element order", "[array]") {
    Array<int> a { 1, 2, 3, 4 };
    a.reverse();
    REQUIRE(a[0] == 4);
    REQUIRE(a[1] == 3);
    REQUIRE(a[2] == 2);
    REQUIRE(a[3] == 1);
}

// ---------------------------------------------------------------------------
// extend / assign
// ---------------------------------------------------------------------------
TEST_CASE("Array extend appends elements from another array", "[array]") {
    Array<int> a { 1, 2 };
    Array<int> b { 3, 4, 5 };
    REQUIRE(a.extend(b));
    REQUIRE(a.length() == 5u);
    REQUIRE(a[4] == 5);
}

TEST_CASE("Array assign clears then extends", "[array]") {
    Array<int> a { 1, 2 };
    Array<int> b { 9, 8, 7 };
    REQUIRE(a.assign(cr::move(b)));
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == 9);
}

// ---------------------------------------------------------------------------
// Move constructor / move assignment
// ---------------------------------------------------------------------------
TEST_CASE("Array move constructor transfers ownership", "[array]") {
    Array<int> a { 1, 2, 3 };
    Array<int> b(cr::move(a));
    REQUIRE(b.length() == 3u);
    REQUIRE(b[0] == 1);
    REQUIRE(a.empty());
}

TEST_CASE("Array move assignment transfers ownership", "[array]") {
    Array<int> a { 10, 20 };
    Array<int> b;
    b = cr::move(a);
    REQUIRE(b.length() == 2u);
    REQUIRE(b[0] == 10);
    REQUIRE(a.empty());
}

// ---------------------------------------------------------------------------
// data / begin / end (range-for)
// ---------------------------------------------------------------------------
TEST_CASE("Array data returns pointer to underlying buffer", "[array]") {
    Array<int> a { 1, 2, 3 };
    REQUIRE(a.data() != nullptr);
    REQUIRE(a.data()[0] == 1);
}

TEST_CASE("Array range-based for loop iterates all elements", "[array]") {
    Array<int> a { 10, 20, 30 };
    int sum = 0;
    for (auto v : a) {
        sum += v;
    }
    REQUIRE(sum == 60);
}

// ---------------------------------------------------------------------------
// set
// ---------------------------------------------------------------------------
TEST_CASE("Array set assigns value at arbitrary index", "[array]") {
    Array<int> a { 1, 2, 3 };
    REQUIRE(a.set(1u, 99));
    REQUIRE(a[1] == 99);
}

// ---------------------------------------------------------------------------
// insert(at, Array) with empty source
// ---------------------------------------------------------------------------
TEST_CASE("Array insert from empty array is a no-op success", "[array]") {
    Array<int> a { 1, 2, 3 };
    Array<int> empty;
    REQUIRE(a.insert(1u, static_cast<const decltype(a)&>(empty)));
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == 1);
    REQUIRE(a[1] == 2);
    REQUIRE(a[2] == 3);
}

// ---------------------------------------------------------------------------
// insert shift with non-trivial element type
// ---------------------------------------------------------------------------
TEST_CASE("Array insert at front with non-trivial type preserves all elements", "[array]") {
    // String has a destructor/move constructor — exercises the construct-vs-assign
    // split in the shift loop.
    Array<String> a;
    a.push(String("b"));
    a.push(String("c"));

    String v("a");
    REQUIRE(a.insert(0u, v));
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == "a");
    REQUIRE(a[1] == "b");
    REQUIRE(a[2] == "c");
}

TEST_CASE("Array insert multiple elements at front with non-trivial type", "[array]") {
    Array<String> a;
    a.push(String("c"));
    a.push(String("d"));

    String items[2] = { String("a"), String("b") };
    REQUIRE(a.insert(0u, items, 2u));
    REQUIRE(a.length() == 4u);
    REQUIRE(a[0] == "a");
    REQUIRE(a[1] == "b");
    REQUIRE(a[2] == "c");
    REQUIRE(a[3] == "d");
}

// ---------------------------------------------------------------------------
// SmallArray typedef
// ---------------------------------------------------------------------------
TEST_CASE("SmallArray pre-allocates 64-element capacity", "[array]") {
    SmallArray<int> sa;
    REQUIRE(sa.capacity() >= 64u);
    REQUIRE(sa.empty());
}

// ---------------------------------------------------------------------------
// insert with count=0 edge case
// ---------------------------------------------------------------------------
TEST_CASE("Array insert with count=0 returns true without modification", "[array]") {
    Array<int> a { 1, 2, 3 };
    int items[] = { 99 };
    REQUIRE(a.insert(0u, items, 0u));
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == 1);
}

// ---------------------------------------------------------------------------
// erase with trivially copyable type uses memmove
// ---------------------------------------------------------------------------
TEST_CASE("Array erase on trivially copyable type works correctly", "[array]") {
    Array<int> a { 1, 2, 3, 4, 5 };
    REQUIRE(a.erase(1u, 2u));
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == 1);
    REQUIRE(a[1] == 4);
    REQUIRE(a[2] == 5);
}

TEST_CASE("Array erase at start with trivially copyable type", "[array]") {
    Array<int> a { 10, 20, 30, 40 };
    REQUIRE(a.erase(0u, 2u));
    REQUIRE(a.length() == 2u);
    REQUIRE(a[0] == 30);
    REQUIRE(a[1] == 40);
}

// ---------------------------------------------------------------------------
// shuffle
// ---------------------------------------------------------------------------
TEST_CASE("Array shuffle changes element order", "[array]") {
    Array<int> a;
    for (int i = 0; i < 100; ++i) {
        a.push(i);
    }

    Array<int> original;
    for (int v : a) {
        original.push(v);
    }

    a.shuffle();
    REQUIRE(a.length() == 100u);

    bool different = false;
    for (size_t i = 0; i < a.length(); ++i) {
        if (a[i] != original[i]) {
            different = true;
            break;
        }
    }
    REQUIRE(different);
}

TEST_CASE("Array shuffle preserves all elements", "[array]") {
    Array<int> a { 1, 2, 3, 4, 5 };
    a.shuffle();
    REQUIRE(a.length() == 5u);

    int sum = 0;
    for (int v : a) {
        sum += v;
    }
    REQUIRE(sum == 15);
}

// ---------------------------------------------------------------------------
// random
// ---------------------------------------------------------------------------
TEST_CASE("Array random returns element from array", "[array]") {
    Array<int> a { 10, 20, 30, 40, 50 };
    int val = a.random();
    bool found = false;
    for (int v : a) {
        if (v == val) {
            found = true;
            break;
        }
    }
    REQUIRE(found);
}

TEST_CASE("Array random on single element returns that element", "[array]") {
    Array<int> a { 42 };
    REQUIRE(a.random() == 42);
}

TEST_CASE("Array random const version", "[array]") {
    const Array<int> a { 1, 2, 3 };
    int val = a.random();
    REQUIRE((val >= 1 && val <= 3));
}

// ---------------------------------------------------------------------------
// index
// ---------------------------------------------------------------------------
TEST_CASE("Array index returns correct index of element", "[array]") {
    Array<int> a { 10, 20, 30 };
    REQUIRE(a.index(a[0]) == 0u);
    REQUIRE(a.index(a[1]) == 1u);
    REQUIRE(a.index(a[2]) == 2u);
}

// ---------------------------------------------------------------------------
// insert self-insertion check
// ---------------------------------------------------------------------------
TEST_CASE("Array insert from self returns false", "[array]") {
    Array<int> a { 1, 2, 3 };
    const Array<int> &ref = a;
    REQUIRE_FALSE(a.insert(1u, ref));
    REQUIRE(a.length() == 3u);
}

// ---------------------------------------------------------------------------
// length with custom return type
// ---------------------------------------------------------------------------
TEST_CASE("Array length with custom return type", "[array]") {
    Array<int> a { 1, 2, 3, 4, 5 };
    REQUIRE(a.length<int>() == 5);
    REQUIRE(a.length<int32_t>() == 5);
    REQUIRE(a.length<size_t>() == 5u);
}

// ---------------------------------------------------------------------------
// shrink edge cases
// ---------------------------------------------------------------------------
TEST_CASE("Array shrink returns false when length equals capacity", "[array]") {
    Array<int> a;
    a.push(1);
    a.shrink();
    REQUIRE_FALSE(a.shrink());
}

TEST_CASE("Array shrink returns false when empty", "[array]") {
    Array<int> a;
    a.reserve(10u);
    a.clear();
    REQUIRE_FALSE(a.shrink());
}

// ---------------------------------------------------------------------------
// insert with nullptr
// ---------------------------------------------------------------------------
TEST_CASE("Array insert with nullptr returns false", "[array]") {
    Array<int> a { 1, 2, 3 };
    REQUIRE_FALSE(a.insert(0u, static_cast<int*>(nullptr), 5u));
    REQUIRE(a.length() == 3u);
}

// ---------------------------------------------------------------------------
// move assignment self-assignment
// ---------------------------------------------------------------------------
TEST_CASE("Array move assignment self-assignment is safe", "[array]") {
    Array<int> a { 1, 2, 3 };
    a = cr::move(a);
    REQUIRE(a.length() == 3u);
    REQUIRE(a[0] == 1);
    REQUIRE(a[1] == 2);
    REQUIRE(a[2] == 3);
}

// ---------------------------------------------------------------------------
// ReservePolicy::Single growth path
// ---------------------------------------------------------------------------
TEST_CASE("Array with ReservePolicy::Single grows correctly", "[array]") {
    Array<int, ReservePolicy::Single> a;
    for (int i = 0; i < 20; ++i) {
        REQUIRE(a.push(i));
    }
    REQUIRE(a.length() == 20u);
    for (int i = 0; i < 20; ++i) {
        REQUIRE(a[i] == i);
    }
}

TEST_CASE("Array with ReservePolicy::Single reserve", "[array]") {
    Array<int, ReservePolicy::Single> a;
    REQUIRE(a.reserve(10u));
    REQUIRE(a.capacity() >= 10u);
    REQUIRE(a.length() == 0u);
}

// ---------------------------------------------------------------------------
// Array with initial size template parameter
// ---------------------------------------------------------------------------
TEST_CASE("Array with initial size template parameter pre-allocates", "[array]") {
    Array<int, ReservePolicy::Multiple, 32> a;
    REQUIRE(a.capacity() >= 32u);
    REQUIRE(a.empty());
}

// ---------------------------------------------------------------------------
// erase with non-trivial type
// ---------------------------------------------------------------------------
TEST_CASE("Array erase with non-trivial type calls destructors", "[array]") {
    Array<String> a;
    a.push(String("one"));
    a.push(String("two"));
    a.push(String("three"));
    a.push(String("four"));
    
    REQUIRE(a.erase(1u, 2u));
    REQUIRE(a.length() == 2u);
    REQUIRE(a[0] == "one");
    REQUIRE(a[1] == "four");
}
