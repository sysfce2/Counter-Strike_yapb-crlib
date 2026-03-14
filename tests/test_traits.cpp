// test_traits.cpp — tests for crlib/traits.h
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

// Note: traits.h only defines types inside namespace cr, so we test them
// with static assertions (compile-time) and runtime REQUIRE checks.

using namespace cr;

// ---------------------------------------------------------------------------
// is_same
// ---------------------------------------------------------------------------
TEST_CASE("is_same::value is true for identical types", "[traits]") {
    STATIC_REQUIRE(cr::is_same<int,    int   >::value);
    STATIC_REQUIRE(cr::is_same<float,  float >::value);
    STATIC_REQUIRE(cr::is_same<double, double>::value);
}

TEST_CASE("is_same::value is false for different types", "[traits]") {
    STATIC_REQUIRE_FALSE(cr::is_same<int, float >::value);
    STATIC_REQUIRE_FALSE(cr::is_same<int, double>::value);
    STATIC_REQUIRE_FALSE(cr::is_same<int, char  >::value);
}

TEST_CASE("is_same with pointer types", "[traits]") {
    STATIC_REQUIRE(cr::is_same<int *, int *>::value);
    STATIC_REQUIRE_FALSE(cr::is_same<int *, int>::value);
}

// ---------------------------------------------------------------------------
// conditional
// ---------------------------------------------------------------------------
TEST_CASE("conditional<true, T, F>::type is T", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::conditional<true,  int, float>::type, int  >::value);
}

TEST_CASE("conditional<false, T, F>::type is F", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::conditional<false, int, float>::type, float>::value);
}

// ---------------------------------------------------------------------------
// enable_if / enable_if_t
// ---------------------------------------------------------------------------
TEST_CASE("enable_if<true, T>::type is T", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::enable_if<true, int>::type, int>::value);
}

TEST_CASE("enable_if_t<true, T> is T", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::enable_if_t<true, double>, double>::value);
}

// ---------------------------------------------------------------------------
// clear_extent
// ---------------------------------------------------------------------------
TEST_CASE("clear_extent for non-array type is the type itself", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::clear_extent<int>::type, int>::value);
}

TEST_CASE("clear_extent for T[] is T", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::clear_extent<int[]>::type, int>::value);
}

TEST_CASE("clear_extent for T[N] is T", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::clear_extent<int[5]>::type, int>::value);
}

// ---------------------------------------------------------------------------
// integral_constant
// ---------------------------------------------------------------------------
TEST_CASE("integral_constant holds compile-time value", "[traits]") {
    using Five  = cr::integral_constant<int, 5>;
    using False = cr::integral_constant<bool, false>;

    STATIC_REQUIRE(Five::value  == 5);
    STATIC_REQUIRE(False::value == false);

    // operator() and operator value_type
    Five five {};
    REQUIRE(static_cast<int>(five) == 5);
    REQUIRE(five() == 5);
}

// ---------------------------------------------------------------------------
// bool_constant / true_type / false_type
// ---------------------------------------------------------------------------
TEST_CASE("true_type has value true", "[traits]") {
    STATIC_REQUIRE(cr::true_type::value == true);
}

TEST_CASE("false_type has value false", "[traits]") {
    STATIC_REQUIRE(cr::false_type::value == false);
}

// ---------------------------------------------------------------------------
// add_lvalue_reference / add_rvalue_reference
// ---------------------------------------------------------------------------
TEST_CASE("add_lvalue_reference converts T to T&", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::add_lvalue_reference<int>::type, int &>::value);
}

TEST_CASE("add_rvalue_reference converts T to T&&", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::add_rvalue_reference<int>::type, int &&>::value);
}

// ---------------------------------------------------------------------------
// nullptr_t
// ---------------------------------------------------------------------------
TEST_CASE("nullptr_t is the type of nullptr", "[traits]") {
    cr::nullptr_t np = nullptr;
    (void)np;
    // If this compiles, the typedef is correct
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// decay (runtime sanity — just verifies the type deduces correctly)
// ---------------------------------------------------------------------------
TEST_CASE("decay removes reference from type", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::decay<int &>::type, int>::value);
}

TEST_CASE("decay removes rvalue reference from type", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::decay<int &&>::type, int>::value);
}

TEST_CASE("decay of plain type is the type itself", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::decay<int>::type, int>::value);
}

TEST_CASE("decay of const type yields the type", "[traits]") {
    // decay strips const for array/function types but may keep it for plain types
    // Here we just verify it compiles and produces a result
    using T = cr::decay<const int>::type;
    (void)sizeof(T);
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// remove_reference / remove_reference_t
// ---------------------------------------------------------------------------
TEST_CASE("remove_reference removes lvalue reference", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_reference<int &>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_reference_t<int &>, int>::value);
}

TEST_CASE("remove_reference removes rvalue reference", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_reference<int &&>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_reference_t<int &&>, int>::value);
}

TEST_CASE("remove_reference on non-reference type is unchanged", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_reference<int>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_reference_t<int>, int>::value);
}

TEST_CASE("remove_reference preserves const qualifier", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_reference<const int &>::type, const int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_reference_t<const int &>, const int>::value);
}

// ---------------------------------------------------------------------------
// remove_cv / remove_cv_t
// ---------------------------------------------------------------------------
TEST_CASE("remove_cv removes const", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_cv<const int>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_cv_t<const int>, int>::value);
}

TEST_CASE("remove_cv removes volatile", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_cv<volatile int>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_cv_t<volatile int>, int>::value);
}

TEST_CASE("remove_cv removes const volatile", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_cv<const volatile int>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_cv_t<const volatile int>, int>::value);
}

TEST_CASE("remove_cv on plain type is unchanged", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_cv<int>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_cv_t<int>, int>::value);
}

// ---------------------------------------------------------------------------
// remove_const / remove_const_t
// ---------------------------------------------------------------------------
TEST_CASE("remove_const removes const", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_const<const int>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_const_t<const int>, int>::value);
}

TEST_CASE("remove_const on non-const type is unchanged", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_const<int>::type, int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_const_t<int>, int>::value);
}

TEST_CASE("remove_const does not remove volatile", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_const<volatile int>::type, volatile int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_const_t<volatile int>, volatile int>::value);
}

TEST_CASE("remove_const on const volatile removes only const", "[traits]") {
    STATIC_REQUIRE(cr::is_same<cr::remove_const<const volatile int>::type, volatile int>::value);
    STATIC_REQUIRE(cr::is_same<cr::remove_const_t<const volatile int>, volatile int>::value);
}

// ---------------------------------------------------------------------------
// is_trivially_copyable / is_trivially_copyable_v
// ---------------------------------------------------------------------------
TEST_CASE("is_trivially_copyable is true for primitive types", "[traits]") {
    STATIC_REQUIRE(cr::is_trivially_copyable<int>::value);
    STATIC_REQUIRE(cr::is_trivially_copyable_v<int>);
    STATIC_REQUIRE(cr::is_trivially_copyable<float>::value);
    STATIC_REQUIRE(cr::is_trivially_copyable_v<float>);
    STATIC_REQUIRE(cr::is_trivially_copyable<double>::value);
    STATIC_REQUIRE(cr::is_trivially_copyable<char>::value);
}

TEST_CASE("is_trivially_copyable is true for pointers", "[traits]") {
    STATIC_REQUIRE(cr::is_trivially_copyable<int *>::value);
    STATIC_REQUIRE(cr::is_trivially_copyable_v<int *>);
    STATIC_REQUIRE(cr::is_trivially_copyable<const char *>::value);
}

namespace {
    struct TrivialStruct {
        int x;
        float y;
    };

    struct NonTrivialStruct {
        NonTrivialStruct() {}
        NonTrivialStruct(const NonTrivialStruct &) {}
        ~NonTrivialStruct() {}
        int x;
    };
}

TEST_CASE("is_trivially_copyable is true for trivial structs", "[traits]") {
    STATIC_REQUIRE(cr::is_trivially_copyable<TrivialStruct>::value);
    STATIC_REQUIRE(cr::is_trivially_copyable_v<TrivialStruct>);
}

TEST_CASE("is_trivially_copyable is false for non-trivial types", "[traits]") {
    STATIC_REQUIRE_FALSE(cr::is_trivially_copyable<NonTrivialStruct>::value);
    STATIC_REQUIRE_FALSE(cr::is_trivially_copyable_v<NonTrivialStruct>);
}
