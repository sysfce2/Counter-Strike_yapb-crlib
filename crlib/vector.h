// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/mathlib.h>
#include <crlib/traits.h>

CR_NAMESPACE_BEGIN

// 3dmath vector
template <typename T> class Vec3D {
public:
#if defined(CR_CXX_MSVC)
#  pragma warning(push)
#  pragma warning(disable: 4201)
#endif
   union {
      struct {
         T x, y, z;
      };
      T data[3] {};
   };
#if defined(CR_CXX_MSVC)
#  pragma warning(pop)
#endif

public:
   constexpr Vec3D () : x {}, y {}, z {} {}

   constexpr Vec3D (const T &scaler) : x (scaler), y (scaler), z (scaler) {}

   constexpr Vec3D (const T &x, const T &y, const T &z) : x (x), y (y), z (z) {}

   constexpr Vec3D (const T *rhs) : x (rhs[0]), y (rhs[1]), z (rhs[2]) {}

#if defined(CR_HAS_SIMD)
   constexpr Vec3D (const SimdVec3Wrap &rhs) : x (rhs.x), y (rhs.y), z (rhs.z) {}
#endif

   constexpr Vec3D (const Vec3D &) = default;

   constexpr Vec3D (nullptr_t) : x {}, y {}, z {} {}

public:
   constexpr operator T * () {
      return data;
   }

   constexpr operator const T * () const {
      return data;
   }

   constexpr Vec3D operator + (const Vec3D &rhs) const {
      return { x + rhs.x, y + rhs.y, z + rhs.z };
   }

   constexpr Vec3D operator - (const Vec3D &rhs) const {
      return { x - rhs.x, y - rhs.y, z - rhs.z };
   }

   constexpr Vec3D operator - () const {
      return { -x, -y, -z };
   }

   friend constexpr Vec3D operator * (const T &scale, const Vec3D &rhs) {
      return { rhs.x * scale, rhs.y * scale, rhs.z * scale };
   }

   constexpr Vec3D operator * (const T &scale) const {
      return { scale * x, scale * y, scale * z };
   }

   constexpr Vec3D operator / (const T &rhs) const {
      const auto inv = T { 1 } / (rhs + static_cast <T> (kFloatEqualEpsilon));
      return { inv * x, inv * y, inv * z };
   }

   // cross product
   constexpr Vec3D operator ^ (const Vec3D &rhs) const {
      return { y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x };
   }

   // dot product
   constexpr T operator | (const Vec3D &rhs) const {
      return x * rhs.x + y * rhs.y + z * rhs.z;
   }

   constexpr Vec3D &operator += (const Vec3D &rhs) {
      x += rhs.x;
      y += rhs.y;
      z += rhs.z;

      return *this;
   }

   constexpr Vec3D &operator -= (const Vec3D &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      z -= rhs.z;

      return *this;
   }

   constexpr Vec3D &operator *= (const T &rhs) {
      x *= rhs;
      y *= rhs;
      z *= rhs;

      return *this;
   }

   constexpr Vec3D &operator /= (const T &rhs) {
      const auto inv = T { 1 } / (rhs + static_cast <T> (kFloatEqualEpsilon));

      x *= inv;
      y *= inv;
      z *= inv;

      return *this;
   }

   constexpr bool operator == (const Vec3D &rhs) const {
      return cr::fequal (x, rhs.x) && cr::fequal (y, rhs.y) && cr::fequal (z, rhs.z);
   }

   constexpr bool operator != (const Vec3D &rhs) const {
      return !operator == (rhs);
   }

   constexpr const T &operator [] (const int i) const {
      return data[i];
   }

   constexpr T &operator [] (const int i) {
      return data[i];
   }

   constexpr void operator = (nullptr_t) {
      clear ();
   }

   Vec3D &operator = (const Vec3D &) = default;

public:
   T length () const {
#if defined(CR_HAS_SIMD)
      return SimdVec3Wrap { x, y, z }.hypot ();
#else
      return cr::sqrtf (lengthSq ());
#endif
   }

   constexpr T lengthSq () const {
      return cr::sqrf (x) + cr::sqrf (y) + cr::sqrf (z);
   }

   T length2d () const {
#if defined(CR_HAS_SIMD)
      return SimdVec3Wrap { x, y }.hypot ();
#else
      return cr::sqrtf (lengthSq2d ());
#endif
   }

   constexpr T lengthSq2d () const {
      return cr::sqrf (x) + cr::sqrf (y);
   }

   T distance (const Vec3D &rhs) const {
      return (*this - rhs).length ();
   }

   T distance2d (const Vec3D &rhs) const {
      return (*this - rhs).length2d ();
   }

   T distanceSq (const Vec3D &rhs) const {
      return (*this - rhs).lengthSq ();
   }

   T distanceSq2d (const Vec3D &rhs) const {
      return (*this - rhs).lengthSq2d ();
   }

   constexpr Vec3D get2d () const {
      return { x, y, T {} };
   }

   Vec3D normalize () const {
#if defined(CR_HAS_SIMD)
      return SimdVec3Wrap { x, y, z }.normalize ();
#else
      auto len = length ();

      if (cr::fzero (len)) {
         return { T {}, T {}, static_cast <T> (kFloatEpsilon) };
      }
      len = T { 1 } / len;
      return { x * len, y * len, z * len };
#endif
   }

   Vec3D normalize2d () const {
      auto len = length2d ();

      if (cr::fzero (len)) {
         return { T {}, static_cast <T> (kFloatEpsilon), T {} };
      }
      len = T { 1 } / len;
      return { x * len, y * len, T {} };
   }

   Vec3D normalize_apx () const {
      const auto len = cr::rsqrtf (lengthSq () + static_cast <T> (kFloatEpsilon));
      return { x * len, y * len, z * len };
   }

   Vec3D normalize2d_apx () const {
      const auto len = cr::rsqrtf (lengthSq2d () + static_cast <T> (kFloatEpsilon));
      return { x * len, y * len, T {} };
   }

   T normalizeInPlace () {
      const auto len = length ();

      if (cr::fzero (len)) {
         *this = { T {}, T {}, static_cast <T> (kFloatEpsilon) };
      }
      else {
         const auto mul = T { 1 } / len;
         *this = { x * mul, y * mul, z * mul };
      }
      return len;
   }

   constexpr bool empty () const {
      return cr::fzero (x) && cr::fzero (y) && cr::fzero (z);
   }

   constexpr void clear () {
      x = y = z = T {};
   }

   Vec3D &clampAngles () {
      x = cr::wrapAngle (x);
      y = cr::wrapAngle (y);
      z = T {};

      return *this;
   }

   // converts a spatial location determined by the vector passed into an absolute X angle (pitch) from the origin of the world.
   T pitch () const {
      if (cr::fzero (z)) {
         return T {};
      }
      return cr::rad2deg (cr::atan2f (z, length2d ()));
   }

   // converts a spatial location determined by the vector passed into an absolute Y angle (yaw) from the origin of the world.
   T yaw () const {
      if (cr::fzero (x) && cr::fzero (y)) {
         return T {};
      }
      return cr::rad2deg (cr::atan2f (y, x));
   }

   // converts a spatial location determined by the vector passed in into constant absolute angles from the origin of the world.
   Vec3D angles () const {
      if (cr::fzero (x) && cr::fzero (y)) {
         return { z > T {} ? T { 90 } : T { 270 }, T {}, T {} };
      }
      return { cr::rad2deg (cr::atan2f (z, length2d ())), cr::rad2deg (cr::atan2f (y, x)), T {} };
   }

   // builds a 3D referential from a view angle, that is to say, the relative "forward", "right" and "upward" direction
   // that a player would have if he were facing this view angle. World angles are stored in Vector structs too, the
   // "x" component corresponding to the X angle (horizontal angle), and the "y" component corresponding to the Y angle
   // (vertical angle).
   void angleVectors (Vec3D *forward, Vec3D *right, Vec3D *upward) const {
#if defined(CR_HAS_SIMD_SSE)
      SimdVec3Wrap { x, y, z }.angleVectors <Vec3D> (forward, right, upward);
#elif defined(CR_HAS_SIMD_NEON) || defined(CR_HAS_SIMD_RVV)
      SimdVec3Wrap s, c;
      SimdVec3Wrap { x, y, z }.angleVectors (s, c);

      if (forward) {
         *forward = { c.x * c.y, c.x * s.y, -s.x };
      }

      if (right) {
         *right = { -s.z * s.x * c.y + c.z * s.y, -s.z * s.x * s.y - c.z * c.y, -s.z * c.x };
      }

      if (upward) {
         *upward = { c.z * s.x * c.y + s.z * s.y, c.z * s.x * s.y - s.z * c.y, c.z * c.x };
      }
#else
      Vec3D s, c;
      const Vec3D r { cr::deg2rad (x), cr::deg2rad (y), cr::deg2rad (z) };

      cr::sincosf (r.x, s.x, c.x);
      cr::sincosf (r.y, s.y, c.y);
      cr::sincosf (r.z, s.z, c.z);

      if (forward) {
         *forward = { c.x * c.y, c.x * s.y, -s.x };
      }

      if (right) {
         *right = { -s.z * s.x * c.y + c.z * s.y, -s.z * s.x * s.y - c.z * c.y, -s.z * c.x };
      }

      if (upward) {
         *upward = { c.z * s.x * c.y + s.z * s.y, c.z * s.x * s.y - s.z * c.y, c.z * c.x };
      }
#endif
   }

   Vec3D forward () const {
      Vec3D fwd {};
      angleVectors (&fwd, nullptr, nullptr);
      return fwd;
   }

   Vec3D upward () const {
      Vec3D up {};
      angleVectors (nullptr, nullptr, &up);
      return up;
   }

   Vec3D right () const {
      Vec3D rt {};
      angleVectors (nullptr, &rt, nullptr);
      return rt;
   }

public:
   static bool bboxIntersects (const Vec3D &min1, const Vec3D &max1, const Vec3D &min2, const Vec3D &max2) {
      return min1.x < max2.x && max1.x > min2.x && min1.y < max2.y && max1.y > min2.y && min1.z < max2.z && max1.z > min2.z;
   }
};

// default is float
using Vector = Vec3D <float>;

CR_NAMESPACE_END
