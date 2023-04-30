//
// YaPB - Counter-Strike Bot based on PODBot by Markus Klinge.
// Copyright © 2004-2022 YaPB Project <yapb@jeefo.net>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/math.h>

CR_NAMESPACE_BEGIN

// 3dmath vector
template <typename T> class Vec3D {
public:
   T x {};
   T y {};
   T z {};

public:
   constexpr Vec3D (const T &scaler = 0.0f) : x (scaler), y (scaler), z (scaler) {}

   constexpr Vec3D (const T &x, const T &y, const T &z) : x (x), y (y), z (z) {}

   constexpr Vec3D (T *rhs) : x (rhs[0]), y (rhs[1]), z (rhs[2]) {}

#if defined (CR_HAS_SSE)
   constexpr Vec3D (const SimdVec3Wrap &rhs) : x (rhs.x), y (rhs.y), z (rhs.z) {}
#endif

   constexpr Vec3D (const Vec3D &) = default;

   constexpr Vec3D (decltype (nullptr)) {
      clear ();
   }

public:
   constexpr operator T *() {
      return &x;
   }

   constexpr operator const T *() const {
      return &x;
   }

   constexpr decltype (auto) operator + (const Vec3D &rhs) const {
      return Vec3D { x + rhs.x, y + rhs.y, z + rhs.z };
   }

   constexpr decltype (auto) operator - (const Vec3D &rhs) const {
      return Vec3D { x - rhs.x, y - rhs.y, z - rhs.z };
   }

   constexpr decltype (auto) operator - () const {
      return Vec3D { -x, -y, -z };
   }

   friend constexpr decltype (auto) operator * (const T &scale, const Vec3D &rhs) {
      return Vec3D { rhs.x * scale, rhs.y * scale, rhs.z * scale };
   }

   constexpr decltype (auto) operator * (const T &scale) const {
      return Vec3D { scale * x, scale * y, scale * z };
   }

   constexpr decltype (auto) operator / (const T &rhs) const {
      const auto inv = 1 / (rhs + kFloatEqualEpsilon);
      return Vec3D { inv * x, inv * y, inv * z };
   }

   // cross product
   constexpr decltype (auto) operator ^ (const Vec3D &rhs) const {
      return Vec3D { y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x };
   }

   // dot product
   constexpr T operator | (const Vec3D &rhs) const {
      return x * rhs.x + y * rhs.y + z * rhs.z;
   }

   constexpr decltype (auto) operator += (const Vec3D &rhs) {
      x += rhs.x;
      y += rhs.y;
      z += rhs.z;

      return *this;
   }

   constexpr decltype (auto) operator -= (const Vec3D &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      z -= rhs.z;

      return *this;
   }

   constexpr decltype (auto) operator *= (const T &rhs) {
      x *= rhs;
      y *= rhs;
      z *= rhs;

      return *this;
   }

   constexpr decltype (auto) operator /= (const T &rhs) {
      const auto inv = 1 / (rhs + kFloatEqualEpsilon);

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

   constexpr void operator = (decltype (nullptr)) {
      clear ();
   }

   constexpr const float &operator [] (const int i) const {
      return &(x)[i];
   }

   constexpr float &operator [] (const int i) {
      return &(x)[i];
   }

   Vec3D &operator = (const Vec3D &) = default;

public:
   T length () const {
      return cr::sqrtf (lengthSq ());
   }

   T lengthSq () const {
      return cr::sqrf (x) + cr::sqrf (y) + cr::sqrf (z);
   }

   T length2d () const {
      return cr::sqrtf (lengthSq2d ());
   }

   T lengthSq2d () const {
      return cr::sqrf (x) + cr::sqrf (y);
   }

   T distance (const Vec3D &rhs) const {
      return apx::sqrtf ((*this - rhs).lengthSq ());
   }

   T distance2d (const Vec3D &rhs) const {
      return apx::sqrtf ((*this - rhs).lengthSq2d ());
   }

   T distanceSq (const Vec3D &rhs) const {
      return (*this - rhs).lengthSq ();
   }

   constexpr decltype (auto) get2d () const {
      return Vec3D { x, y, 0.0f };
   }

   Vec3D normalize () const {
#if defined (CR_HAS_SSE)
      return SimdVec3Wrap { x, y, z }.normalize ();
#else
      auto len = length () + cr::kFloatEpsilon;

      if (cr::fzero (len)) {
         return { 0.0f, 0.0f, 1.0f };
      }
      len = 1.0f / len;
      return { x * len, y * len, z * len };
#endif
   }

   Vec3D normalize2d () const {
#if defined (CR_HAS_SSE)
      return SimdVec3Wrap { x, y }.normalize ();
#else
      auto len = length2d () + cr::kFloatEpsilon;

      if (cr::fzero (len)) {
         return { 0.0f, 1.0f, 0.0f };
      }
      len = 1.0f / len;
      return { x * len, y * len, 0.0f };
#endif
   }

   Vec3D normalize_apx () const {
      const float length = cr::rsqrtf (lengthSq () + kFloatEpsilon);
      return { x * length, y * length, z * length };
   }

   Vec3D normalize2d_apx () const {
      const float length = cr::rsqrtf (lengthSq2d () + kFloatEpsilon);
      return { x * length, y * length, 0.0f };
   }

   constexpr bool empty () const {
      return cr::fzero (x) && cr::fzero (y) && cr::fzero (z);
   }

   constexpr void clear () {
      x = y = z = 0.0f;
   }

   decltype (auto) clampAngles () {
      x = cr::wrapAngle (x);
      y = cr::wrapAngle (y);
      z = 0.0f;

      return *this;
   }

   T pitch () const {
      if (cr::fzero (z)) {
         return 0.0f;
      }
      return cr::deg2rad (cr::atan2f (z, length2d ()));
   }

   T yaw () const {
      if (cr::fzero (x) && cr::fzero (y)) {
         return 0.0f;
      }
      return cr::rad2deg (cr::atan2f (y, x));
   }

   Vec3D angles () const {
      if (cr::fzero (x) && cr::fzero (y)) {
         return { z > 0.0f ? 90.0f : 270.0f, 0.0, 0.0f };
      }
      return { cr::rad2deg (cr::atan2f (z, length2d ())), cr::rad2deg (cr::atan2f (y, x)), 0.0f };
   }

   void angleVectors (Vec3D *forward, Vec3D *right, Vec3D *upward) const {
#if defined (CR_HAS_SSE) && !defined (CR_ARCH_ARM32)
      static SimdVec3Wrap s, c;
      SimdVec3Wrap { x, y, z }.angleVectors (s, c);
#else
      static Vec3D s, c, r;
      r = { cr::deg2rad (x), cr::deg2rad (y), cr::deg2rad (z) };

      cr::sincosf (r.x, s.x, c.x);
      cr::sincosf (r.y, s.y, c.y);
      cr::sincosf (r.z, s.z, c.z);
#endif

      if (forward) {
         *forward = { c.x * c.y, c.x * s.y, -s.x };
      }

      if (right) {
         *right = { -s.z * s.x * c.y + c.z * s.y, -s.z * s.x * s.y - c.z * c.y, -s.z * c.x };
      }

      if (upward) {
         *upward = { c.z * s.x * c.y + s.z * s.y, c.z * s.x * s.y - s.z * c.y, c.z * c.x };
      }
   }

   const Vec3D &forward () {
      static Vec3D s_fwd {};
      angleVectors (&s_fwd, nullptr, nullptr);

      return s_fwd;
   }

   const Vec3D &upward () {
      static Vec3D s_up {};
      angleVectors (nullptr, nullptr, &s_up);

      return s_up;
   }

   const Vec3D &right () {
      static Vec3D s_right {};
      angleVectors (nullptr, &s_right, nullptr);

      return s_right;
   }
};

// default is float
using Vector = Vec3D <float>;

CR_NAMESPACE_END
