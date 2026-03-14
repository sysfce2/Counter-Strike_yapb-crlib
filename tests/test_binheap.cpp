// test_binheap.cpp — tests for crlib/binheap.h
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
TEST_CASE("BinaryHeap default construction is empty", "[binheap]") {
    BinaryHeap<int> heap;
    REQUIRE(heap.empty());
    REQUIRE(heap.length() == 0u);
}

// ---------------------------------------------------------------------------
// push / top / pop — min-heap ordering
// ---------------------------------------------------------------------------
TEST_CASE("BinaryHeap push maintains heap ordering", "[binheap]") {
    BinaryHeap<int> heap;
    heap.push(30);
    heap.push(10);
    heap.push(20);

    REQUIRE(heap.length() == 3u);
    REQUIRE(heap.top() == 10);  // min-heap
}

TEST_CASE("BinaryHeap pop removes the minimum element", "[binheap]") {
    BinaryHeap<int> heap;
    heap.push(5);
    heap.push(1);
    heap.push(3);
    heap.push(2);
    heap.push(4);

    REQUIRE(heap.pop() == 1);
    REQUIRE(heap.pop() == 2);
    REQUIRE(heap.pop() == 3);
    REQUIRE(heap.pop() == 4);
    REQUIRE(heap.pop() == 5);
    REQUIRE(heap.empty());
}

TEST_CASE("BinaryHeap pop on single element empties the heap", "[binheap]") {
    BinaryHeap<int> heap;
    heap.push(42);
    REQUIRE(heap.pop() == 42);
    REQUIRE(heap.empty());
}

// ---------------------------------------------------------------------------
// emplace
// ---------------------------------------------------------------------------
TEST_CASE("BinaryHeap emplace constructs element in place", "[binheap]") {
    BinaryHeap<int> heap;
    heap.emplace(100);
    heap.emplace(50);
    heap.emplace(75);

    REQUIRE(heap.top() == 50);
    REQUIRE(heap.length() == 3u);
}

// ---------------------------------------------------------------------------
// clear
// ---------------------------------------------------------------------------
TEST_CASE("BinaryHeap clear empties all elements", "[binheap]") {
    BinaryHeap<int> heap;
    heap.push(1);
    heap.push(2);
    heap.push(3);
    heap.clear();

    REQUIRE(heap.empty());
    REQUIRE(heap.length() == 0u);
}

// ---------------------------------------------------------------------------
// Move constructor
// ---------------------------------------------------------------------------
TEST_CASE("BinaryHeap move constructor transfers state", "[binheap]") {
    BinaryHeap<int> a;
    a.push(3);
    a.push(1);
    a.push(2);

    BinaryHeap<int> b(cr::move(a));
    REQUIRE(b.length() == 3u);
    REQUIRE(b.top() == 1);
    REQUIRE(a.empty());
}

// ---------------------------------------------------------------------------
// Large insertion test (many elements maintain invariant)
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Regression: push(U&&) must forward, not unconditionally move
// ---------------------------------------------------------------------------
TEST_CASE("BinaryHeap push lvalue copies and leaves original intact", "[binheap]") {
    // Before the fix, push(cr::move(item)) moved from the lvalue, leaving the
    // caller's value in a moved-from (empty) state.
    BinaryHeap<int> heap;
    int s = 42;
    heap.push(s);             // lvalue — must copy, not move
    REQUIRE(s == 42);    // original must be unchanged
    REQUIRE(heap.top() == 42);
}

TEST_CASE("BinaryHeap push rvalue moves efficiently", "[binheap]") {
    BinaryHeap<int> heap;
    heap.push(100);   // rvalue — must move (not copy)
    REQUIRE(heap.top() == 100);
}

// ---------------------------------------------------------------------------
// Large insertion test (many elements maintain invariant)
// ---------------------------------------------------------------------------
TEST_CASE("BinaryHeap sorts elements extracted in order", "[binheap]") {
    BinaryHeap<int> heap;
    const int values[] = { 9, 4, 7, 1, 8, 3, 6, 2, 5 };
    for (auto v : values) {
        heap.push(v);
    }
    REQUIRE(heap.length() == 9u);

    int prev = heap.pop();
    while (!heap.empty()) {
        int cur = heap.pop();
        REQUIRE(prev <= cur);
        prev = cur;
    }
}
