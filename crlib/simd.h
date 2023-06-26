//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>

#if defined (CR_HAS_SIMD_SSE)
#  if defined (CR_ARCH_ARM)
#     include <crlib/simd/sse2neon.h>
#  else
#     include <smmintrin.h>
#  endif
#elif defined (CR_HAS_SIMD_NEON)
#  include <arm_neon.h>
#endif

namespace cr::simd {
#if defined (CR_HAS_SIMD_SSE)
#  include <crlib/simd/sse2.h>
#elif defined (CR_HAS_SIMD_NEON)
#  include <crlib/simd/neon.h>
#endif
}

CR_NAMESPACE_BEGIN

#if defined (CR_HAS_SIMD)

#if defined (CR_HAS_SIMD_SSE)
CR_SIMD_ALIGNED const auto simd_EPS = _mm_set1_ps (kFloatEpsilon);
#else
CR_SIMD_ALIGNED const auto simd_EPS = vdupq_n_f32 (kFloatEpsilon);
#endif

// forward declaration of vector
template <typename T> class Vec3D;

// simple wrapper for vector
class CR_SIMD_ALIGNED SimdVec3Wrap final {
private:
#if defined (CR_HAS_SIMD_SSE)
   template <bool AVOID_NAN = false> static CR_SIMD_TARGET_AIL ("sse4.1") __m128 simd_dot4 (__m128 v0, __m128 v1) {
      if (cpuflags.sse41) {
         if constexpr (AVOID_NAN) {
            return _mm_dp_ps (v0, v1, 0x7f);
         }
         return _mm_dp_ps (v0, v1, 0x71);
      }

      auto mul0 = _mm_mul_ps (v0, v1);
      auto had0 = _mm_add_ps (_mm_movehl_ps (mul0, mul0), _mm_hadd_ps (mul0, mul0));

      if constexpr (AVOID_NAN) {
         had0 = _mm_add_ps (had0, simd_EPS); // avoid NaN's
      }
      return had0;
   }
#else
   template <bool AVOID_NAN = false> static CR_FORCE_INLINE float32x4_t simd_dot4 (float32x4_t v0, float32x4_t v1) {
      auto mul0 = vmulq_f32 (v0, v1);
      auto sum1 = vaddq_f32 (mul0, vrev64q_f32 (mul0));
      auto sum2 = vaddq_f32 (sum1, vcombine_f32 (vget_high_f32 (sum1), vget_low_f32 (sum1)));

      if constexpr (AVOID_NAN) {
         sum2 = vaddq_f32 (sum2, simd_EPS); // avoid NaN's
      }
      return sum2;
}
#endif

public:
#if defined (CR_CXX_MSVC)
#   pragma warning(push)
#   pragma warning(disable: 4201)
#endif
   union {
#if defined (CR_HAS_SIMD_SSE)
      __m128 m { _mm_setzero_ps () };
#else
      float32x4_t m { vdupq_n_f32 (0) };
#endif
      struct {
         float x, y, z, w;
      };
   };

#if defined (CR_CXX_MSVC)
#   pragma warning(pop) 
#endif

   SimdVec3Wrap (const float *data) {
#if defined(CR_HAS_SIMD_SSE)
      m = _mm_loadu_ps (data);
#else
      m = vld1q_f32 (data);
#endif
   }

   SimdVec3Wrap (const float &x, const float &y) {
#if defined(CR_HAS_SIMD_SSE)
      m = _mm_set_ps (0.0f, 0.0f, y, x);
#else
      float CR_SIMD_ALIGNED data[4] = { 0.0f, 0.0f, y, x };
      m = vld1q_f32 (data);
#endif
   }

#if defined(CR_HAS_SIMD_SSE)
   constexpr SimdVec3Wrap (__m128 m) : m (m) {}
#else
   constexpr SimdVec3Wrap (float32x4_t m) : m (m) {}
#endif

public:
   constexpr SimdVec3Wrap () : x (0.0f), y (0.0f), z (0.0f), w (0.0f) {}
   ~SimdVec3Wrap () = default;

public:
#if defined (CR_HAS_SIMD_SSE)
   CR_SIMD_TARGET ("sse4.1")
   SimdVec3Wrap normalize () const {
      return { _mm_div_ps (m, _mm_sqrt_ps (simd_dot4 <true> (m, m))) };
   }

   CR_SIMD_TARGET ("sse4.1")
   float hypot () const {
      return _mm_cvtss_f32 (_mm_sqrt_ps (simd_dot4 (m, m)));
   }

   CR_SIMD_TARGET ("sse4.1")
   float hypot2d () const {
      __m128 mul0 = _mm_mul_ps (m, m);
      return _mm_cvtss_f32 (_mm_sqrt_ps (_mm_hadd_ps (mul0, mul0)));
   }

   CR_SIMD_TARGET ("sse4.1")
   float dot (SimdVec3Wrap rhs) const {
      return _mm_cvtss_f32 (simd_dot4 (m, rhs.m));
   }

   CR_FORCE_INLINE SimdVec3Wrap cross (SimdVec3Wrap rhs) const {
      return _mm_sub_ps (
         _mm_mul_ps (_mm_shuffle_ps (m, m, _MM_SHUFFLE (3, 0, 2, 1)), _mm_shuffle_ps (rhs.m, rhs.m, _MM_SHUFFLE (3, 1, 0, 2))),
         _mm_mul_ps (_mm_shuffle_ps (m, m, _MM_SHUFFLE (3, 1, 0, 2)), _mm_shuffle_ps (rhs.m, rhs.m, _MM_SHUFFLE (3, 0, 2, 1)))
      );
   }

   // this function directly taken from rehlds project https://github.com/dreamstalker/rehlds
   template <typename T> CR_FORCE_INLINE void angleVectors (T *forward, T *right, T *upward) {
      static constexpr CR_SIMD_ALIGNED float kDegToRad[] = {
         kDegreeToRadians, kDegreeToRadians,
         kDegreeToRadians, kDegreeToRadians
      };

      static constexpr CR_SIMD_ALIGNED float kNeg03[4] = {
         -0.0, 0.0f, 0.0f, -0.0f
      };

      __m128 cos0, sin0;
      simd::sincos_ps (_mm_mul_ps (m, _mm_load_ps (kDegToRad)), cos0, sin0);

      auto shf0 = _mm_shuffle_ps (sin0, cos0, _MM_SHUFFLE (2, 1, 0, 0));
      auto shf1 = _mm_shuffle_ps (sin0, sin0, _MM_SHUFFLE (0, 0, 2, 1));
      auto mul0 = _mm_mul_ps (shf0, shf1);

      shf0 = _mm_shuffle_ps (sin0, cos0, _MM_SHUFFLE (0, 1, 1, 1));
      shf1 = _mm_shuffle_ps (cos0, sin0, _MM_SHUFFLE (2, 2, 0, 0));
      shf0 = _mm_shuffle_ps (shf0, shf0, _MM_SHUFFLE (3, 0, 2, 0));

      auto shf2 = _mm_shuffle_ps (cos0, cos0, _MM_SHUFFLE (1, 0, 2, 2));
      shf2 = _mm_mul_ps (shf2, _mm_mul_ps (shf0, shf1));

      shf1 = _mm_shuffle_ps (cos0, sin0, _MM_SHUFFLE (1, 2, 1, 1));
      shf0 = _mm_shuffle_ps (sin0, cos0, _MM_SHUFFLE (2, 2, 1, 2));
      shf1 = _mm_shuffle_ps (shf1, shf1, _MM_SHUFFLE (3, 1, 2, 0));

      shf0 = _mm_xor_ps (shf0, _mm_load_ps (kNeg03));
      shf0 = _mm_mul_ps (shf0, shf1);

      shf2 = _mm_add_ps (shf2, shf0);

      if (forward) {
         _mm_storel_pi (reinterpret_cast <__m64 *> (forward->data), _mm_shuffle_ps (mul0, mul0, _MM_SHUFFLE (0, 0, 2, 0)));
         forward->data[2] = -_mm_cvtss_f32 (cos0);
      }

      if (right) {
         auto shf3 = _mm_shuffle_ps (shf2, mul0, _MM_SHUFFLE (3, 3, 1, 0));
         shf3 = _mm_xor_ps (shf3, _mm_set1_ps (kNeg03[0]));

         _mm_storel_pi (reinterpret_cast <__m64 *> (right->data), shf3);
         _mm_store_ss (&right->data[2], _mm_shuffle_ps (shf3, shf3, _MM_SHUFFLE (0, 0, 0, 2)));
      }

      if (upward) {
         _mm_storel_pi (reinterpret_cast <__m64 *> (upward->data), _mm_shuffle_ps (shf2, shf2, _MM_SHUFFLE (0, 0, 3, 2)));
         upward->data[2] = _mm_cvtss_f32 (_mm_shuffle_ps (mul0, mul0, _MM_SHUFFLE (0, 0, 0, 1)));
      }
   }
#else
   CR_FORCE_INLINE SimdVec3Wrap normalize () const {
      return { simd::div_ps (m, simd::sqrt_ps (simd_dot4 <true> (m, m))) };
   }

   CR_FORCE_INLINE float hypot () const {
      return vgetq_lane_f32 (simd::sqrt_ps (simd_dot4 (m, m)), 0);
   }

   CR_FORCE_INLINE void angleVectors (SimdVec3Wrap &sines, SimdVec3Wrap &cosines) {
      static constexpr CR_SIMD_ALIGNED float kDegToRad[] = {
         kDegreeToRadians, kDegreeToRadians,
         kDegreeToRadians, kDegreeToRadians
      };
      simd::sincos_ps (vmulq_f32 (m, vld1q_f32 (kDegToRad)), sines.m, cosines.m);
   }
#endif
};

#endif

CR_NAMESPACE_END
