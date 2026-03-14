// test_memory.cpp — tests for crlib/memory.h (Memory class)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// mem::allocate / mem::release
// ---------------------------------------------------------------------------
TEST_CASE("mem::allocate allocates non-null memory", "[memory]") {
    int *p = mem::allocate<int>();
    REQUIRE(p != nullptr);
    mem::release(p);
}

TEST_CASE("mem::allocate allocates an array of N elements", "[memory]") {
    int *arr = mem::allocate<int>(10u);
    REQUIRE(arr != nullptr);
    // Write and read back to verify the allocation is usable
    for (int i = 0; i < 10; ++i) {
        arr[i] = i * 2;
    }
    for (int i = 0; i < 10; ++i) {
        REQUIRE(arr[i] == i * 2);
    }
    mem::release(arr);
}

// ---------------------------------------------------------------------------
// mem::construct / mem::destruct
// ---------------------------------------------------------------------------
TEST_CASE("mem::construct placement-new on raw memory", "[memory]") {
    int *p = mem::allocate<int>();
    mem::construct(p, 42);
    REQUIRE(*p == 42);
    mem::destruct(p);
    mem::release(p);
}

TEST_CASE("mem::construct calls constructor with arguments", "[memory]") {
    struct Pair {
        int a, b;
        Pair(int a, int b) : a(a), b(b) {}
        ~Pair() = default;
    };

    Pair *p = mem::allocate<Pair>();
    mem::construct(p, 3, 7);
    REQUIRE(p->a == 3);
    REQUIRE(p->b == 7);
    mem::destruct(p);
    mem::release(p);
}

TEST_CASE("mem::destruct calls destructor", "[memory]") {
    static int destructCount = 0;

    struct Tracker {
        int id;
        Tracker(int i) : id(i) {}
        ~Tracker() { ++destructCount; }
    };

    Tracker *p = mem::allocate<Tracker>();
    mem::construct(p, 1);
    REQUIRE(destructCount == 0);
    mem::destruct(p);
    REQUIRE(destructCount == 1);
    mem::release(p);
}

// ---------------------------------------------------------------------------
// mem::allocateAndConstruct
// ---------------------------------------------------------------------------
TEST_CASE("mem::allocateAndConstruct allocates and constructs in one call", "[memory]") {
    int *p = mem::allocateAndConstruct<int>(99);
    REQUIRE(p != nullptr);
    REQUIRE(*p == 99);
    mem::destruct(p);
    mem::release(p);
}

TEST_CASE("mem::allocateAndConstruct with struct", "[memory]") {
    struct Point {
        float x, y;
        Point(float x, float y) : x(x), y(y) {}
    };

    Point *pt = mem::allocateAndConstruct<Point>(1.0f, 2.0f);
    REQUIRE(pt->x == Catch::Approx(1.0f));
    REQUIRE(pt->y == Catch::Approx(2.0f));
    mem::destruct(pt);
    mem::release(pt);
}

// ---------------------------------------------------------------------------
// mem::transfer — move-constructs from src to dst
// ---------------------------------------------------------------------------
TEST_CASE("mem::transfer moves elements from src to dst", "[memory]") {
    const size_t n = 5;
    int *src = mem::allocate<int>(n);
    int *dst = mem::allocate<int>(n);

    for (size_t i = 0; i < n; ++i) {
        mem::construct(&src[i], static_cast<int>(i * 10));
    }
    mem::transfer(dst, src, n);

    for (size_t i = 0; i < n; ++i) {
        REQUIRE(dst[i] == static_cast<int>(i * 10));
    }

    for (size_t i = 0; i < n; ++i) {
        mem::destruct(&dst[i]);
    }
    mem::release(src);
    mem::release(dst);
}

TEST_CASE("mem::transfer moves non-trivial objects", "[memory]") {
    struct Box {
        int value;
        Box() : value(0) {}
        Box(int v) : value(v) {}
        Box(Box &&rhs) noexcept : value(rhs.value) { rhs.value = -1; }
        Box &operator=(Box &&rhs) noexcept { value = rhs.value; rhs.value = -1; return *this; }
        ~Box() = default;
    };

    const size_t n = 3;
    Box *src = mem::allocate<Box>(n);
    Box *dst = mem::allocate<Box>(n);

    for (size_t i = 0; i < n; ++i) {
        mem::construct(&src[i], static_cast<int>(i));
    }
    mem::transfer(dst, src, n);

    REQUIRE(dst[0].value == 0);
    REQUIRE(dst[1].value == 1);
    REQUIRE(dst[2].value == 2);

    for (size_t i = 0; i < n; ++i) {
        mem::destruct(&dst[i]);
    }
    mem::release(src);
    mem::release(dst);
}

// ---------------------------------------------------------------------------
// mem::constructArray / mem::destructArray
// ---------------------------------------------------------------------------
TEST_CASE("mem::constructArray constructs array of objects", "[memory]") {
    static int constructCount = 0;
    
    struct Counter {
        int id;
        Counter() : id(0) { ++constructCount; }
        Counter(int i) : id(i) { ++constructCount; }
        ~Counter() = default;
    };
    
    constructCount = 0;
    const size_t n = 5;
    Counter *arr = mem::allocate<Counter>(n);
    mem::constructArray(arr, n);
    
    REQUIRE(constructCount == 5);
    for (size_t i = 0; i < n; ++i) {
        REQUIRE(arr[i].id == 0);
    }
    
    mem::destructArray(arr, n);
    mem::release(arr);
}

TEST_CASE("mem::constructArray with arguments", "[memory]") {
    struct Value {
        int v;
        Value(int x) : v(x) {}
        ~Value() = default;
    };
    
    const size_t n = 4;
    Value *arr = mem::allocate<Value>(n);
    mem::constructArray(arr, n, 42);
    
    for (size_t i = 0; i < n; ++i) {
        REQUIRE(arr[i].v == 42);
    }
    
    mem::destructArray(arr, n);
    mem::release(arr);
}

TEST_CASE("mem::destructArray calls destructor on each element", "[memory]") {
    static int destructCount = 0;
    
    struct Tracker {
        Tracker() = default;
        ~Tracker() { ++destructCount; }
    };
    
    destructCount = 0;
    const size_t n = 3;
    Tracker *arr = mem::allocate<Tracker>(n);
    mem::constructArray(arr, n);
    
    REQUIRE(destructCount == 0);
    mem::destructArray(arr, n);
    REQUIRE(destructCount == 3);
    
    mem::release(arr);
}

// ---------------------------------------------------------------------------
// mem::release returns nullptr
// ---------------------------------------------------------------------------
TEST_CASE("mem::release returns nullptr", "[memory]") {
    int *p = mem::allocate<int>();
    int *result = mem::release(p);
    REQUIRE(result == nullptr);
}
