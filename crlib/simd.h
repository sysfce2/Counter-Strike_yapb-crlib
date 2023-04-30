//
// YaPB - Counter-Strike Bot based on PODBot by Markus Klinge.
// Copyright © 2004-2022 YaPB Project <yapb@jeefo.net>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>

#if defined (CR_HAS_SSE)
#  include CR_INTRIN_INCLUDE
#  if defined (CR_ARCH_ARM)
#     include <crlib/ssemath/sincos_arm.h>
#endif
#endif

CR_NAMESPACE_BEGIN

#if defined (CR_HAS_SSE)

namespace ssemath {
#  include <crlib/ssemath/ssemath.h>
}

CR_ALIGN16 const auto simd_EPS = _mm_set1_ps (kFloatEpsilon);

// simple wrapper for vector
class CR_ALIGN16 SimdVec3Wrap final {
private:
   template <bool ADD_EPS = false> static __m128 _mm_dot4_ps (__m128 v0, __m128 v1) {
      v0 = _mm_mul_ps (v0, v1);

      v1 = _mm_shuffle_ps (v0, v0, _MM_SHUFFLE (2, 3, 0, 1));
      v0 = _mm_add_ps (v0, v1);
      v1 = _mm_shuffle_ps (v0, v0, _MM_SHUFFLE (0, 1, 2, 3));
      v0 = _mm_add_ps (v0, v1);

      if constexpr (ADD_EPS) {
         v0 = _mm_add_ps (v0, simd_EPS); // avoid NaN's
      }
      return v0;
   }

public:
#if defined (CR_CXX_MSVC)
#   pragma warning(push)
#   pragma warning(disable: 4201)
#endif
   union {
      __m128 m { };

      struct {
         float x, y, z, w;
      };
   };

#if defined (CR_CXX_MSVC)
#   pragma warning(pop) 
#endif

   SimdVec3Wrap (const float &x, const float &y, const float &z) {
      m = _mm_set_ps (0.0f, z, y, x);
   }

   SimdVec3Wrap (const float &x, const float &y) {
      m = _mm_set_ps (0.0f, 0.0f, y, x);
   }

   constexpr SimdVec3Wrap (__m128 m) : m (m) {}

public:
   constexpr SimdVec3Wrap () : x (0.0f), y (0.0f), z (0.0f), w (0.0f) {}
   ~SimdVec3Wrap () = default;

public:
   SimdVec3Wrap normalize () const {
      return { _mm_div_ps (m, _mm_sqrt_ps (_mm_dot4_ps <true> (m, m))) };
   }

   float hypot () const {
      return _mm_cvtss_f32 (_mm_sqrt_ps (_mm_dot4_ps (m, m)));
   }

   float dot (SimdVec3Wrap rhs) const {
      return _mm_cvtss_f32 (_mm_dot4_ps (m, rhs.m));
   }

   SimdVec3Wrap cross (SimdVec3Wrap rhs) const {
      return _mm_sub_ps (
         _mm_mul_ps (_mm_shuffle_ps (m, m, _MM_SHUFFLE (3, 0, 2, 1)), _mm_shuffle_ps (rhs.m, rhs.m, _MM_SHUFFLE (3, 1, 0, 2))),
         _mm_mul_ps (_mm_shuffle_ps (m, m, _MM_SHUFFLE (3, 1, 0, 2)), _mm_shuffle_ps (rhs.m, rhs.m, _MM_SHUFFLE (3, 0, 2, 1)))
      );
   }

   void angleVectors (SimdVec3Wrap &sines, SimdVec3Wrap &cosines) {
      static constexpr CR_ALIGN16 float d2r[] = {
         kDegreeToRadians, kDegreeToRadians,
         kDegreeToRadians, kDegreeToRadians
      };
#if defined (CR_ARCH_ARM64)
      neon_sincos_ps (_mm_mul_ps (m, _mm_load_ps (d2r)), sines.m, cosines.m);
#else
      ssemath::sincos_ps (_mm_mul_ps (m, _mm_load_ps (d2r)), sines.m, cosines.m);
#endif
   }
};

#endif

CR_NAMESPACE_END
