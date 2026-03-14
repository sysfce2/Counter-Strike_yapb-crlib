// test_uniqueptr.cpp — tests for crlib/uniqueptr.h
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Single-object UniquePtr
// ---------------------------------------------------------------------------
TEST_CASE("UniquePtr default construction holds nullptr", "[uniqueptr]") {
    UniquePtr<int> p;
    REQUIRE_FALSE(bool(p));
    REQUIRE(p.get() == nullptr);
}

TEST_CASE("UniquePtr construction from raw pointer holds value", "[uniqueptr]") {
    UniquePtr<int> p(new int(42));
    REQUIRE(bool(p));
    REQUIRE(p.get() != nullptr);
    REQUIRE(*p == 42);
}

TEST_CASE("UniquePtr move constructor transfers ownership", "[uniqueptr]") {
    UniquePtr<int> a(new int(7));
    UniquePtr<int> b(cr::move(a));

    REQUIRE(bool(b));
    REQUIRE(*b == 7);
    REQUIRE_FALSE(bool(a));
    REQUIRE(a.get() == nullptr);
}

TEST_CASE("UniquePtr move assignment transfers ownership", "[uniqueptr]") {
    UniquePtr<int> a(new int(99));
    UniquePtr<int> b;
    b = cr::move(a);

    REQUIRE(bool(b));
    REQUIRE(*b == 99);
    REQUIRE_FALSE(bool(a));
}

TEST_CASE("UniquePtr nullptr assignment releases resource", "[uniqueptr]") {
    UniquePtr<int> p(new int(5));
    p = nullptr;
    REQUIRE_FALSE(bool(p));
    REQUIRE(p.get() == nullptr);
}

TEST_CASE("UniquePtr release returns raw pointer and relinquishes ownership", "[uniqueptr]") {
    UniquePtr<int> p(new int(10));
    int *raw = p.release();

    REQUIRE(raw != nullptr);
    REQUIRE(*raw == 10);
    REQUIRE_FALSE(bool(p));

    delete raw;  // we now own it
}

TEST_CASE("UniquePtr reset replaces the managed resource", "[uniqueptr]") {
    UniquePtr<int> p(new int(1));
    p.reset(new int(2));
    REQUIRE(*p == 2);

    p.reset();
    REQUIRE_FALSE(bool(p));
}

TEST_CASE("UniquePtr arrow operator accesses struct members", "[uniqueptr]") {
    struct Foo { int x { 55 }; };
    UniquePtr<Foo> p(new Foo{});
    REQUIRE(p->x == 55);
    p->x = 77;
    REQUIRE((*p).x == 77);
}

// ---------------------------------------------------------------------------
// Array specialisation UniquePtr<T[]>
// ---------------------------------------------------------------------------
TEST_CASE("UniquePtr<T[]> default construction holds nullptr", "[uniqueptr]") {
    UniquePtr<int[]> p;
    REQUIRE_FALSE(bool(p));
}

TEST_CASE("UniquePtr<T[]> supports element access", "[uniqueptr]") {
    UniquePtr<int[]> p(new int[3]{ 10, 20, 30 });
    REQUIRE(bool(p));
    REQUIRE(p[0] == 10);
    REQUIRE(p[1] == 20);
    REQUIRE(p[2] == 30);
}

TEST_CASE("UniquePtr<T[]> move constructor transfers ownership", "[uniqueptr]") {
    UniquePtr<int[]> a(new int[2]{ 4, 5 });
    UniquePtr<int[]> b(cr::move(a));

    REQUIRE(bool(b));
    REQUIRE(b[0] == 4);
    REQUIRE_FALSE(bool(a));
}

TEST_CASE("UniquePtr<T[]> reset clears the array", "[uniqueptr]") {
    UniquePtr<int[]> p(new int[4]{});
    p.reset();
    REQUIRE_FALSE(bool(p));
}

// ---------------------------------------------------------------------------
// makeUnique factory
// ---------------------------------------------------------------------------
TEST_CASE("makeUnique creates a UniquePtr for a single object", "[uniqueptr]") {
    auto p = makeUnique<int>(123);
    REQUIRE(bool(p));
    REQUIRE(*p == 123);
}

TEST_CASE("makeUnique creates a UniquePtr for an array", "[uniqueptr]") {
    auto p = makeUnique<int[]>(5u);
    REQUIRE(bool(p));
    // Array is value-initialised to 0
    for (size_t i = 0; i < 5; ++i) {
        REQUIRE(p[i] == 0);
    }
}

TEST_CASE("makeUnique with struct calls constructor", "[uniqueptr]") {
    struct Bar {
        int a, b;
        Bar(int a, int b) : a(a), b(b) {}
    };
    auto p = makeUnique<Bar>(3, 7);
    REQUIRE(p->a == 3);
    REQUIRE(p->b == 7);
}
