// test_deque.cpp — tests for crlib/deque.h
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Construction / empty state
// ---------------------------------------------------------------------------
TEST_CASE("Deque default construction is empty", "[deque]") {
    Deque<int> d;
    REQUIRE(d.empty());
    REQUIRE(d.length() == 0u);
}

// ---------------------------------------------------------------------------
// emplaceLast / emplaceFront
// ---------------------------------------------------------------------------
TEST_CASE("Deque emplaceLast appends to the back", "[deque]") {
    Deque<int> d;
    d.emplaceLast(10);
    d.emplaceLast(20);
    d.emplaceLast(30);

    REQUIRE(d.length() == 3u);
    REQUIRE(d.front()  == 10);
    REQUIRE(d.last()   == 30);
}

TEST_CASE("Deque emplaceFront prepends to the front", "[deque]") {
    Deque<int> d;
    d.emplaceFront(30);
    d.emplaceFront(20);
    d.emplaceFront(10);

    REQUIRE(d.length() == 3u);
    REQUIRE(d.front()  == 10);
    REQUIRE(d.last()   == 30);
}

TEST_CASE("Deque mixed emplace front and back", "[deque]") {
    Deque<int> d;
    d.emplaceLast(2);
    d.emplaceFront(1);
    d.emplaceLast(3);

    REQUIRE(d.length() == 3u);
    REQUIRE(d.front()  == 1);
    REQUIRE(d.last()   == 3);
}

// ---------------------------------------------------------------------------
// popFront / popLast
// ---------------------------------------------------------------------------
TEST_CASE("Deque popFront removes and returns front element", "[deque]") {
    Deque<int> d;
    d.emplaceLast(1);
    d.emplaceLast(2);
    d.emplaceLast(3);

    REQUIRE(d.popFront() == 1);
    REQUIRE(d.length()   == 2u);
    REQUIRE(d.front()    == 2);
}

TEST_CASE("Deque popLast removes and returns back element", "[deque]") {
    Deque<int> d;
    d.emplaceLast(1);
    d.emplaceLast(2);
    d.emplaceLast(3);

    REQUIRE(d.popLast() == 3);
    REQUIRE(d.length()  == 2u);
    REQUIRE(d.last()    == 2);
}

// ---------------------------------------------------------------------------
// discardFront / discardLast
// ---------------------------------------------------------------------------
TEST_CASE("Deque discardFront removes front without returning it", "[deque]") {
    Deque<int> d;
    d.emplaceLast(5);
    d.emplaceLast(6);
    d.discardFront();

    REQUIRE(d.length() == 1u);
    REQUIRE(d.front()  == 6);
}

TEST_CASE("Deque discardLast removes back without returning it", "[deque]") {
    Deque<int> d;
    d.emplaceLast(5);
    d.emplaceLast(6);
    d.discardLast();

    REQUIRE(d.length() == 1u);
    REQUIRE(d.last()   == 5);
}

// ---------------------------------------------------------------------------
// clear
// ---------------------------------------------------------------------------
TEST_CASE("Deque clear resets length to zero", "[deque]") {
    Deque<int> d;
    d.emplaceLast(1);
    d.emplaceLast(2);
    d.clear();

    REQUIRE(d.empty());
    REQUIRE(d.length() == 0u);
}

// ---------------------------------------------------------------------------
// Move constructor / move assignment
// ---------------------------------------------------------------------------
TEST_CASE("Deque move constructor transfers state", "[deque]") {
    Deque<int> a;
    a.emplaceLast(10);
    a.emplaceLast(20);

    Deque<int> b(cr::move(a));
    REQUIRE(b.length() == 2u);
    REQUIRE(b.front()  == 10);
    REQUIRE(a.empty());
}

TEST_CASE("Deque move assignment transfers state", "[deque]") {
    Deque<int> a;
    a.emplaceLast(7);
    a.emplaceLast(8);

    Deque<int> b;
    b = cr::move(a);
    REQUIRE(b.length() == 2u);
    REQUIRE(b.last()   == 8);
    REQUIRE(a.empty());
}

// ---------------------------------------------------------------------------
// clear() with non-trivial type (regression: clear() skipped destructors)
// ---------------------------------------------------------------------------
TEST_CASE("Deque clear calls destructors for non-trivial element type", "[deque]") {
    int destructions = 0;

    struct DestructCount {
        int *count;
        explicit DestructCount (int *c) : count (c) {}
        ~DestructCount () { ++(*count); }
    };

    {
        Deque<DestructCount> d;
        d.emplaceLast(&destructions);
        d.emplaceLast(&destructions);
        d.emplaceLast(&destructions);
        REQUIRE(d.length() == 3u);

        d.clear();
        REQUIRE(d.empty());
        REQUIRE(destructions == 3);  // all 3 must be destructed by clear()
    }
    // d's destructor runs on an already-empty deque — must not double-destruct
    REQUIRE(destructions == 3);
}

TEST_CASE("Deque is reusable after clear", "[deque]") {
    Deque<int> d;
    d.emplaceLast(1);
    d.emplaceLast(2);
    d.clear();

    d.emplaceLast(10);
    d.emplaceLast(20);
    REQUIRE(d.length() == 2u);
    REQUIRE(d.front()  == 10);
    REQUIRE(d.last()   == 20);
}

// ---------------------------------------------------------------------------
// Growth — insert many elements to force reallocation
// ---------------------------------------------------------------------------
TEST_CASE("Deque handles large numbers of front/back insertions correctly", "[deque]") {
    Deque<int> d;
    for (int i = 0; i < 100; ++i) {
        d.emplaceLast(i);
    }
    REQUIRE(d.length() == 100u);
    REQUIRE(d.front()  == 0);
    REQUIRE(d.last()   == 99);

    for (int i = 0; i < 100; ++i) {
        REQUIRE(d.popFront() == i);
    }
    REQUIRE(d.empty());
}
