// test_vector.cpp — tests for crlib/vector.h (Vec3D / Vector)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

static constexpr float kEps = 1e-3f;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
TEST_CASE("Vector default construction is zero", "[vector]") {
    Vector v;
    REQUIRE(v.x == Catch::Approx(0.0f));
    REQUIRE(v.y == Catch::Approx(0.0f));
    REQUIRE(v.z == Catch::Approx(0.0f));
}

TEST_CASE("Vector scalar construction fills all components", "[vector]") {
    Vector v(5.0f);
    REQUIRE(v.x == Catch::Approx(5.0f));
    REQUIRE(v.y == Catch::Approx(5.0f));
    REQUIRE(v.z == Catch::Approx(5.0f));
}

TEST_CASE("Vector component construction", "[vector]") {
    Vector v(1.0f, 2.0f, 3.0f);
    REQUIRE(v.x == Catch::Approx(1.0f));
    REQUIRE(v.y == Catch::Approx(2.0f));
    REQUIRE(v.z == Catch::Approx(3.0f));
}

TEST_CASE("Vector from pointer", "[vector]") {
    float arr[3] = { 4.0f, 5.0f, 6.0f };
    Vector v(arr);
    REQUIRE(v.x == Catch::Approx(4.0f));
    REQUIRE(v.y == Catch::Approx(5.0f));
    REQUIRE(v.z == Catch::Approx(6.0f));
}

TEST_CASE("Vector nullptr construction clears", "[vector]") {
    Vector v(1.0f, 2.0f, 3.0f);
    v = nullptr;
    REQUIRE(v.x == Catch::Approx(0.0f));
    REQUIRE(v.y == Catch::Approx(0.0f));
    REQUIRE(v.z == Catch::Approx(0.0f));
}

// ---------------------------------------------------------------------------
// Array / subscript access
// ---------------------------------------------------------------------------
TEST_CASE("Vector data array and operator[] are consistent", "[vector]") {
    Vector v(10.0f, 20.0f, 30.0f);
    REQUIRE(v[0] == Catch::Approx(10.0f));
    REQUIRE(v[1] == Catch::Approx(20.0f));
    REQUIRE(v[2] == Catch::Approx(30.0f));
    REQUIRE(v.data[0] == Catch::Approx(10.0f));
}

// ---------------------------------------------------------------------------
// Arithmetic operators
// ---------------------------------------------------------------------------
TEST_CASE("Vector operator+ adds component-wise", "[vector]") {
    Vector a(1, 2, 3), b(4, 5, 6);
    Vector c = a + b;
    REQUIRE(c.x == Catch::Approx(5.0f));
    REQUIRE(c.y == Catch::Approx(7.0f));
    REQUIRE(c.z == Catch::Approx(9.0f));
}

TEST_CASE("Vector operator- subtracts component-wise", "[vector]") {
    Vector a(5, 7, 9), b(1, 2, 3);
    Vector c = a - b;
    REQUIRE(c.x == Catch::Approx(4.0f));
    REQUIRE(c.y == Catch::Approx(5.0f));
    REQUIRE(c.z == Catch::Approx(6.0f));
}

TEST_CASE("Vector unary minus negates components", "[vector]") {
    Vector v(1, -2, 3);
    Vector neg = -v;
    REQUIRE(neg.x == Catch::Approx(-1.0f));
    REQUIRE(neg.y == Catch::Approx(2.0f));
    REQUIRE(neg.z == Catch::Approx(-3.0f));
}

TEST_CASE("Vector scalar multiplication", "[vector]") {
    Vector v(1, 2, 3);
    Vector scaled = v * 2.0f;
    REQUIRE(scaled.x == Catch::Approx(2.0f));
    REQUIRE(scaled.y == Catch::Approx(4.0f));
    REQUIRE(scaled.z == Catch::Approx(6.0f));

    Vector scaled2 = 3.0f * v;
    REQUIRE(scaled2.x == Catch::Approx(3.0f));
}

TEST_CASE("Vector scalar division", "[vector]") {
    Vector v(4, 6, 8);
    Vector d = v / 2.0f;
    REQUIRE(d.x == Catch::Approx(2.0f).epsilon(kEps));
    REQUIRE(d.y == Catch::Approx(3.0f).epsilon(kEps));
    REQUIRE(d.z == Catch::Approx(4.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// Compound assignment
// ---------------------------------------------------------------------------
TEST_CASE("Vector operator+= accumulates component-wise", "[vector]") {
    Vector v(1, 2, 3);
    v += Vector(10, 20, 30);
    REQUIRE(v.x == Catch::Approx(11.0f));
    REQUIRE(v.y == Catch::Approx(22.0f));
    REQUIRE(v.z == Catch::Approx(33.0f));
}

TEST_CASE("Vector operator-= subtracts component-wise", "[vector]") {
    Vector v(10, 20, 30);
    v -= Vector(1, 2, 3);
    REQUIRE(v.x == Catch::Approx(9.0f));
    REQUIRE(v.y == Catch::Approx(18.0f));
    REQUIRE(v.z == Catch::Approx(27.0f));
}

TEST_CASE("Vector operator*= scales component-wise", "[vector]") {
    Vector v(2, 4, 6);
    v *= 0.5f;
    REQUIRE(v.x == Catch::Approx(1.0f));
    REQUIRE(v.y == Catch::Approx(2.0f));
    REQUIRE(v.z == Catch::Approx(3.0f));
}

// ---------------------------------------------------------------------------
// Dot product (operator|)
// ---------------------------------------------------------------------------
TEST_CASE("Vector dot product (operator|)", "[vector]") {
    Vector a(1, 0, 0), b(0, 1, 0);
    REQUIRE((a | b) == Catch::Approx(0.0f));   // perpendicular

    Vector c(1, 2, 3), d(4, 5, 6);
    REQUIRE((c | d) == Catch::Approx(32.0f));  // 1*4 + 2*5 + 3*6
}

// ---------------------------------------------------------------------------
// Cross product (operator^)
// ---------------------------------------------------------------------------
TEST_CASE("Vector cross product (operator^)", "[vector]") {
    Vector x(1, 0, 0), y(0, 1, 0);
    Vector z = x ^ y;
    REQUIRE(z.x == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(z.y == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(z.z == Catch::Approx(1.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// Equality
// ---------------------------------------------------------------------------
TEST_CASE("Vector operator== uses epsilon comparison", "[vector]") {
    Vector a(1.0f, 2.0f, 3.0f);
    Vector b(1.0f, 2.0f, 3.0f);
    Vector c(1.0f, 2.0f, 4.0f);
    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
    REQUIRE(a != c);
}

// ---------------------------------------------------------------------------
// Length / LengthSq
// ---------------------------------------------------------------------------
TEST_CASE("Vector length computes Euclidean length", "[vector]") {
    Vector v(3.0f, 4.0f, 0.0f);
    REQUIRE(v.length() == Catch::Approx(5.0f).epsilon(kEps));

    Vector unit(1, 0, 0);
    REQUIRE(unit.length() == Catch::Approx(1.0f).epsilon(kEps));
}

TEST_CASE("Vector lengthSq computes squared length", "[vector]") {
    Vector v(1, 2, 2);
    REQUIRE(v.lengthSq() == Catch::Approx(9.0f).epsilon(kEps));
}

TEST_CASE("Vector length2d computes XY length", "[vector]") {
    Vector v(3.0f, 4.0f, 100.0f);
    REQUIRE(v.length2d() == Catch::Approx(5.0f).epsilon(kEps));
}

TEST_CASE("Vector lengthSq2d computes XY squared length", "[vector]") {
    Vector v(3.0f, 4.0f, 100.0f);
    REQUIRE(v.lengthSq2d() == Catch::Approx(25.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// Distance
// ---------------------------------------------------------------------------
TEST_CASE("Vector distance between two points", "[vector]") {
    Vector a(0, 0, 0), b(3, 4, 0);
    REQUIRE(a.distance(b) == Catch::Approx(5.0f).epsilon(kEps));
}

TEST_CASE("Vector distanceSq between two points", "[vector]") {
    Vector a(0, 0, 0), b(3, 4, 0);
    REQUIRE(a.distanceSq(b) == Catch::Approx(25.0f).epsilon(kEps));
}

TEST_CASE("Vector distance2d ignores Z component", "[vector]") {
    Vector a(0, 0, 0), b(3, 4, 100);
    REQUIRE(a.distance2d(b) == Catch::Approx(5.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// Normalize
// ---------------------------------------------------------------------------
TEST_CASE("Vector normalize produces unit vector", "[vector]") {
    Vector v(3.0f, 4.0f, 0.0f);
    Vector n = v.normalize();
    REQUIRE(n.length() == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(n.x == Catch::Approx(0.6f).epsilon(kEps));
    REQUIRE(n.y == Catch::Approx(0.8f).epsilon(kEps));
}

TEST_CASE("Vector normalize2d normalises XY plane only", "[vector]") {
    Vector v(3.0f, 4.0f, 10.0f);
    Vector n = v.normalize2d();
    REQUIRE(n.length2d() == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(n.z == Catch::Approx(0.0f));
}

TEST_CASE("Vector normalizeInPlace modifies in-place and returns length", "[vector]") {
    Vector v(0, 5, 0);
    float len = v.normalizeInPlace();
    REQUIRE(len == Catch::Approx(5.0f).epsilon(kEps));
    REQUIRE(v.length() == Catch::Approx(1.0f).epsilon(kEps));
}

TEST_CASE("Vector normalize_apx approximate normalisation", "[vector]") {
    Vector v(3, 4, 0);
    Vector n = v.normalize_apx();
    REQUIRE(n.length() == Catch::Approx(1.0f).epsilon(0.01f));
}

// ---------------------------------------------------------------------------
// empty / clear
// ---------------------------------------------------------------------------
TEST_CASE("Vector empty detects near-zero vector", "[vector]") {
    Vector zero;
    REQUIRE(zero.empty());

    Vector nonzero(1, 0, 0);
    REQUIRE_FALSE(nonzero.empty());
}

TEST_CASE("Vector clear zeroes all components", "[vector]") {
    Vector v(1, 2, 3);
    v.clear();
    REQUIRE(v.x == Catch::Approx(0.0f));
    REQUIRE(v.y == Catch::Approx(0.0f));
    REQUIRE(v.z == Catch::Approx(0.0f));
    REQUIRE(v.empty());
}

// ---------------------------------------------------------------------------
// get2d
// ---------------------------------------------------------------------------
TEST_CASE("Vector get2d zeroes the Z component", "[vector]") {
    Vector v(1, 2, 3);
    Vector v2d = v.get2d();
    REQUIRE(v2d.x == Catch::Approx(1.0f));
    REQUIRE(v2d.y == Catch::Approx(2.0f));
    REQUIRE(v2d.z == Catch::Approx(0.0f));
}

// ---------------------------------------------------------------------------
// bboxIntersects
// ---------------------------------------------------------------------------
TEST_CASE("Vector bboxIntersects detects overlapping bboxes", "[vector]") {
    Vector min1(0, 0, 0), max1(2, 2, 2);
    Vector min2(1, 1, 1), max2(3, 3, 3);
    REQUIRE(Vector::bboxIntersects(min1, max1, min2, max2));
}

TEST_CASE("Vector bboxIntersects returns false for non-overlapping boxes", "[vector]") {
    Vector min1(0, 0, 0), max1(1, 1, 1);
    Vector min2(5, 5, 5), max2(6, 6, 6);
    REQUIRE_FALSE(Vector::bboxIntersects(min1, max1, min2, max2));
}

// ---------------------------------------------------------------------------
// angles / yaw / pitch
// ---------------------------------------------------------------------------
TEST_CASE("Vector yaw returns zero for unit X vector", "[vector]") {
    Vector v(1.0f, 0.0f, 0.0f);
    REQUIRE(v.yaw() == Catch::Approx(0.0f).epsilon(kEps));
}

TEST_CASE("Vector yaw returns 90 for unit Y vector", "[vector]") {
    Vector v(0.0f, 1.0f, 0.0f);
    REQUIRE(v.yaw() == Catch::Approx(90.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// clampAngles
// ---------------------------------------------------------------------------
TEST_CASE("Vector clampAngles wraps X and Y and zeroes Z", "[vector]") {
    Vector v(200.0f, -200.0f, 45.0f);
    v.clampAngles();
    REQUIRE(v.z == Catch::Approx(0.0f));
    // x and y should be wrapped into [-180, 180)
    REQUIRE(v.x >= -180.0f);
    REQUIRE(v.x <   180.0f);
    REQUIRE(v.y >= -180.0f);
    REQUIRE(v.y <   180.0f);
}

// ===========================================================================
// Additional tests for missing coverage
// ===========================================================================

// ---------------------------------------------------------------------------
// Vector compound division assignment
// ---------------------------------------------------------------------------
TEST_CASE("Vector operator/= divides components by scalar", "[vector]") {
    Vector v(4.0f, 6.0f, 8.0f);
    v /= 2.0f;
    REQUIRE(v.x == Catch::Approx(2.0f).epsilon(kEps));
    REQUIRE(v.y == Catch::Approx(3.0f).epsilon(kEps));
    REQUIRE(v.z == Catch::Approx(4.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// Vector 2D operations
// ---------------------------------------------------------------------------
TEST_CASE("Vector distanceSq2d computes 2D squared distance", "[vector]") {
    Vector a(1.0f, 2.0f, 10.0f);
    Vector b(4.0f, 6.0f, 20.0f); // Z component should be ignored
    float dist2 = a.distanceSq2d(b);
    // (4-1)^2 + (6-2)^2 = 9 + 16 = 25
    REQUIRE(dist2 == Catch::Approx(25.0f));
}

TEST_CASE("Vector normalize2d_apx approximates 2D normalization", "[vector]") {
    Vector v(3.0f, 4.0f, 5.0f);
    Vector norm = v.normalize2d_apx();
    // Should normalize only x and y, z is zeroed in 2D normalization
    float len2d = cr::sqrtf(norm.x * norm.x + norm.y * norm.y);
    REQUIRE(len2d == Catch::Approx(1.0f).epsilon(0.01f)); // Approximate
    REQUIRE(norm.z == Catch::Approx(0.0f).epsilon(kEps)); // Z is zeroed in 2D
}

// ---------------------------------------------------------------------------
// Vector angle-related methods
// ---------------------------------------------------------------------------
TEST_CASE("Vector pitch computes pitch angle", "[vector]") {
    Vector v(0.0f, 0.0f, 1.0f); // Straight up
    float pitch = v.pitch();
    REQUIRE(pitch == Catch::Approx(90.0f).epsilon(kEps));
    
    Vector v2(1.0f, 0.0f, 0.0f); // Horizontal
    float pitch2 = v2.pitch();
    REQUIRE(pitch2 == Catch::Approx(0.0f).epsilon(kEps));
}

TEST_CASE("Vector angles returns pitch and yaw", "[vector]") {
    Vector v(1.0f, 0.0f, 0.0f);
    Vector angles = v.angles();
    REQUIRE(angles.x == Catch::Approx(0.0f).epsilon(kEps)); // pitch
    REQUIRE(angles.y == Catch::Approx(0.0f).epsilon(kEps)); // yaw
    REQUIRE(angles.z == Catch::Approx(0.0f).epsilon(kEps));
    
    Vector v2(0.0f, 0.0f, 1.0f);
    Vector angles2 = v2.angles();
    REQUIRE(angles2.x == Catch::Approx(90.0f).epsilon(kEps)); // pitch up
}

TEST_CASE("Vector angleVectors computes forward/right/up vectors", "[vector]") {
    Vector angles(0.0f, 0.0f, 0.0f); // Looking along +X
    Vector forward, right, upward;
    
    angles.angleVectors(&forward, &right, &upward);
    
    // Forward should be +X
    REQUIRE(forward.x == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(forward.y == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(forward.z == Catch::Approx(0.0f).epsilon(kEps));
    
    // Right should be -Y (right-handed coordinate system)
    REQUIRE(right.x == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(right.y == Catch::Approx(-1.0f).epsilon(kEps));
    REQUIRE(right.z == Catch::Approx(0.0f).epsilon(kEps));
    
    // Up should be +Z
    REQUIRE(upward.x == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(upward.y == Catch::Approx(0.0f).epsilon(kEps));
    REQUIRE(upward.z == Catch::Approx(1.0f).epsilon(kEps));
}

TEST_CASE("Vector forward/upward/right return basis vectors", "[vector]") {
    Vector angles(0.0f, 90.0f, 0.0f); // Looking along +Y
    Vector forward = angles.forward();
    Vector upward = angles.upward();
    Vector right = angles.right();
    
    // Forward should be +Y (looking east)
    REQUIRE(forward.x == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(forward.y == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(forward.z == Catch::Approx(0.0f).margin(1e-5f));
    
    // Right should be +X (south in right-handed system)
    REQUIRE(right.x == Catch::Approx(1.0f).epsilon(kEps));
    REQUIRE(right.y == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(right.z == Catch::Approx(0.0f).margin(1e-5f));
    
    // Up should be +Z (unchanged by yaw)
    REQUIRE(upward.x == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(upward.y == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(upward.z == Catch::Approx(1.0f).epsilon(kEps));
}

// ---------------------------------------------------------------------------
// Vector pointer conversion operators
// ---------------------------------------------------------------------------
TEST_CASE("Vector pointer conversion operators", "[vector]") {
    Vector v(1.0f, 2.0f, 3.0f);
    
    // const pointer conversion
    const Vector& cv = v;
    const float* cptr = cv;
    REQUIRE(cptr[0] == Catch::Approx(1.0f));
    REQUIRE(cptr[1] == Catch::Approx(2.0f));
    REQUIRE(cptr[2] == Catch::Approx(3.0f));
    
    // non-const pointer conversion
    float* ptr = v;
    ptr[1] = 5.0f;
    REQUIRE(v.y == Catch::Approx(5.0f));
}

// ---------------------------------------------------------------------------
// Vector yaw with arbitrary angles
// ---------------------------------------------------------------------------
TEST_CASE("Vector yaw with 45 degree angle", "[vector]") {
    Vector v(1.0f, 1.0f, 0.0f); // 45 degrees in XY plane
    float yaw = v.yaw();
    REQUIRE(yaw == Catch::Approx(45.0f).epsilon(kEps));
}

TEST_CASE("Vector yaw with negative angle", "[vector]") {
    Vector v(1.0f, -1.0f, 0.0f); // -45 degrees in XY plane
    float yaw = v.yaw();
    REQUIRE(yaw == Catch::Approx(-45.0f).epsilon(kEps));
}
