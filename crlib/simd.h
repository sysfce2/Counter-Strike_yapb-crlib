// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/basic.h>

#if defined(CR_HAS_SIMD_RVV)
#  include <cmath>
#endif

#if defined(CR_HAS_SIMD_SSE)
#  include <smmintrin.h>
#elif defined(CR_HAS_SIMD_NEON)
#  include <arm_neon.h>
#elif defined(CR_HAS_SIMD_RVV)
#  include <riscv_vector.h>
#endif

namespace cr::simd {
#if defined(CR_HAS_SIMD_SSE)
#  include <crlib/simd/sse2.h>
#elif defined(CR_HAS_SIMD_NEON)
#  include <crlib/simd/neon.h>
#elif defined(CR_HAS_SIMD_RVV)
#  include <crlib/simd/rvv.h>
#endif
}

#if defined(CR_HAS_SIMD_NEON)
#  define SSE2NEON_SUPPRESS_WARNINGS
#  include <crlib/simd/sse2neon.h>
#endif

CR_NAMESPACE_BEGIN

#if defined(CR_HAS_SIMD)

// forward declaration of vector
template <typename T> class Vec3D;

// simple wrapper for vector
class CR_SIMD_ALIGNED SimdVec3Wrap final {
private:
#if defined(CR_HAS_SIMD_SSE) || defined(CR_HAS_SIMD_NEON)
   static CR_FORCE_INLINE __m128 _simd_load_mask (const uint32_t *mask) {
      return _mm_load_ps (reinterpret_cast <const float *> (mask));
   }

   template <int XmmMask> static CR_SIMD_TARGET_AIL ("sse4.1") __m128 _simd_dpps (__m128 v0, __m128 v1) {
#if defined(CR_HAS_SIMD_SSE)
      if (cpuflags.sse41) {
         return _mm_dp_ps (v0, v1, XmmMask);
      }
      else if (cpuflags.sse3) {
         const auto mul0 = _mm_mul_ps (v0, v1);
         const auto had0 = _mm_hadd_ps (mul0, mul0);
         const auto had1 = _mm_hadd_ps (had0, had0);

         return had1;
      }

      const auto mul0 = _mm_mul_ps (v0, v1);
      const auto shf1 = _mm_shuffle_ps (mul0, mul0, _MM_SHUFFLE (0, 0, 0, 1));
      const auto shf2 = _mm_shuffle_ps (mul0, mul0, _MM_SHUFFLE (0, 0, 0, 2));

      return _mm_add_ss (_mm_add_ss (mul0, shf1), shf2);
#else
      return _mm_dp_ps (v0, v1, XmmMask);
#endif
   }

   static CR_FORCE_INLINE __m128 _simd_div (__m128 v0, __m128 v1) {
      return _mm_andnot_ps (_mm_cmpeq_ps (v1, _mm_setzero_ps ()), _mm_div_ps (v0, v1));
   }
#endif

public:
#if defined(CR_CXX_MSVC)
#  pragma warning(push)
#  pragma warning(disable: 4201)
#endif

#if defined(CR_HAS_SIMD_RVV)
   float CR_SIMD_ALIGNED data[4] {};
   float &x = data[0];
   float &y = data[1];
   float &z = data[2];
   float &w = data[3];
#else
   union {
#if defined(CR_HAS_SIMD_SSE)
      __m128 m { _mm_setzero_ps () };
#else
      float32x4_t m { vdupq_n_f32 (0) };
#endif
      struct {
         float x, y, z, w;
      };
   };
#endif

#if defined(CR_CXX_MSVC)
#  pragma warning(pop)
#endif

   SimdVec3Wrap () = default;
   ~SimdVec3Wrap () = default;

   SimdVec3Wrap (const float &x, const float &y, const float &z) {
#if defined(CR_HAS_SIMD_SSE)
      m = _mm_set_ps (0.0f, z, y, x);
#elif defined(CR_HAS_SIMD_RVV)
      data[0] = x;
      data[1] = y;
      data[2] = z;
      data[3] = 0.0f;
#else
      float CR_SIMD_ALIGNED arr[4] = { x, y, z, 0.0f };
      m = vld1q_f32 (arr);
#endif
   }

   SimdVec3Wrap (const float &x, const float &y) {
#if defined(CR_HAS_SIMD_SSE)
      m = _mm_set_ps (0.0f, 0.0f, y, x);
#elif defined(CR_HAS_SIMD_RVV)
      data[0] = x;
      data[1] = y;
      data[2] = 0.0f;
      data[3] = 0.0f;
#else
      float CR_SIMD_ALIGNED arr[4] = { x, y, 0.0f, 0.0f };
      m = vld1q_f32 (arr);
#endif
   }

#if defined(CR_HAS_SIMD_SSE)
   SimdVec3Wrap (__m128 m) : m (m) {}
#elif defined(CR_HAS_SIMD_RVV)
   SimdVec3Wrap (vfloat32m1_t v) {
      simd::rvv_store4f (data, v);
   }
#else
   SimdVec3Wrap (float32x4_t m) : m (m) {}
#endif

public:
#if defined(CR_HAS_SIMD_RVV)
   SimdVec3Wrap normalize () const {
      vfloat32m1_t v = simd::rvv_load4f (data);
      float dot = simd::rvv_reduce_sum4f (simd::rvv_mul4f (v, v));
      vfloat32m1_t len = simd::rvv_splat4f (::sqrtf (dot));
      vfloat32m1_t result = simd::div_ps (v, len);
      return SimdVec3Wrap (result);
   }

   float hypot () const {
      float dot = x * x + y * y + z * z;
      return ::sqrtf (dot);
   }

   CR_FORCE_INLINE void angleVectors (SimdVec3Wrap &sines, SimdVec3Wrap &cosines) {
      static constexpr CR_SIMD_ALIGNED float kDegToRad[] = {
         kDegreeToRadians, kDegreeToRadians,
         kDegreeToRadians, kDegreeToRadians
      };
      vfloat32m1_t v = simd::rvv_load4f (data);
      vfloat32m1_t rad = simd::rvv_load4f (kDegToRad);
      vfloat32m1_t angles = simd::rvv_mul4f (v, rad);
      vfloat32m1_t s, c;
      simd::sincos_ps (angles, s, c);
      simd::rvv_store4f (sines.data, s);
      simd::rvv_store4f (cosines.data, c);
   }
#elif defined(CR_HAS_SIMD_SSE)
   CR_SIMD_TARGET ("sse4.1")
   SimdVec3Wrap normalize () const {
      return _simd_div (m, _mm_sqrt_ps (_simd_dpps <0xff> (m, m)));
   }

   CR_SIMD_TARGET ("sse4.1")
   float hypot () const {
      return _mm_cvtss_f32 (_mm_sqrt_ps (_simd_dpps <0x71> (m, m)));
   }

   template <typename T> CR_FORCE_INLINE void angleVectors (T *forward, T *right, T *upward) {
      static constexpr CR_SIMD_ALIGNED float kDegToRad[] = {
         kDegreeToRadians, kDegreeToRadians,
         kDegreeToRadians, kDegreeToRadians
      };

      static constexpr CR_SIMD_ALIGNED uint32_t kNegMask_1111[4] = {
         0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u
      };

      static constexpr CR_SIMD_ALIGNED uint32_t kNegMask_1001[4] = {
         0x80000000u, 0u, 0u, 0x80000000u
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

      shf0 = _mm_xor_ps (shf0, _simd_load_mask (kNegMask_1001));
      shf0 = _mm_mul_ps (shf0, shf1);

      shf2 = _mm_add_ps (shf2, shf0);

      if (forward) {
         _mm_storel_pi (reinterpret_cast <__m64 *> (forward->data), _mm_shuffle_ps (mul0, mul0, _MM_SHUFFLE (0, 0, 2, 0)));
         forward->data[2] = -_mm_cvtss_f32 (cos0);
      }

      if (right) {
         auto shf3 = _mm_shuffle_ps (shf2, mul0, _MM_SHUFFLE (3, 3, 1, 0));
         shf3 = _mm_xor_ps (shf3, _simd_load_mask (kNegMask_1111));

         _mm_storel_pi (reinterpret_cast <__m64 *> (right->data), shf3);
         _mm_store_ss (&right->data[2], _mm_shuffle_ps (shf3, shf3, _MM_SHUFFLE (0, 0, 0, 2)));
      }

      if (upward) {
         _mm_storel_pi (reinterpret_cast <__m64 *> (upward->data), _mm_shuffle_ps (shf2, shf2, _MM_SHUFFLE (0, 0, 3, 2)));
         upward->data[2] = _mm_cvtss_f32 (_mm_shuffle_ps (mul0, mul0, _MM_SHUFFLE (0, 0, 0, 1)));
      }
   }
#else
   SimdVec3Wrap normalize () const {
      return _simd_div (m, _mm_sqrt_ps (_simd_dpps <0xff> (m, m)));
   }

   float hypot () const {
      return _mm_cvtss_f32 (_mm_sqrt_ps (_simd_dpps <0x71> (m, m)));
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
