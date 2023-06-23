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

// sse 4.2 implementations borrowed from https://github.com/WojciechMula/simd-string/
class SimdString final : public Singleton <SimdString> {
public:
   explicit SimdString () = default;
   ~SimdString () = default;

private:
   using StrLen = decltype (::strlen);
   using StrCmp = decltype (::strcmp);
   using MemCmp = decltype (::memcmp);
   using StrNCmp = decltype (::strncmp);

public:
   // use default implementations
   StrLen *strlen_ { ::strlen };
   StrCmp *strcmp_ { ::strcmp };
   MemCmp *memcmp_ { ::memcmp };
   StrNCmp *strncmp_ { ::strncmp };

private:
#if defined (CR_HAS_SIMD_SSE)
   static size_t CR_SIMD_TARGET_AIL ("sse4.2") simd_sse42_strlen (const char *s) {
      size_t result = 0;

      auto mem = reinterpret_cast<__m128i *> (const_cast <char *> (s));
      const auto zeros = _mm_setzero_si128 ();

      constexpr uint8_t mode = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_LEAST_SIGNIFICANT;

      for (;; mem++, result += 16) {
         const auto data = _mm_loadu_si128 (mem);

         if (_mm_cmpistrc (data, zeros, mode)) {
            const auto idx = _mm_cmpistri (data, zeros, mode);
            return result + idx;
         }
      }
   }

   static int CR_SIMD_TARGET_AIL ("sse4.2") simd_sse42_memcmp (const void *s1, const void *s2, size_t n) {
      if (n == 0 || s1 == s2) {
         return 0;
      }
      auto ptr1 = reinterpret_cast <__m128i *> (const_cast <void *> (s1));
      auto ptr2 = reinterpret_cast <__m128i *> (const_cast <void *> (s2));

      constexpr auto mode = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

      for (; n != 0; ptr1++, ptr2++) {
         const auto a = _mm_loadu_si128 (ptr1);
         const auto b = _mm_loadu_si128 (ptr2);

         if (_mm_cmpestrc (a, n, b, n, mode)) {
            const auto idx = _mm_cmpestri (a, n, b, n, mode);

            const auto b1 = (reinterpret_cast <char *> (ptr1))[idx];
            const auto b2 = (reinterpret_cast <char *> (ptr2))[idx];

            if (b1 < b2) {
               return -1;
            }
            else if (b1 > b2) {
               return +1;
            }
            else {
               return 0;
            }
         }

         if (n > 16) {
            n -= 16;
         }
         else {
            n = 0;
         }
      }
      return 0;
   }

   static int CR_SIMD_TARGET_AIL ("sse4.2") simd_sse42_strcmp (const char *s1, const char *s2) {
      if (s1 == s2) {
         return 0;
      }
      auto ptr1 = reinterpret_cast<__m128i *> (const_cast <char *> (s1));
      auto ptr2 = reinterpret_cast<__m128i *> (const_cast <char *> (s2));

      constexpr uint8_t mode = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

      for (;; ptr1++, ptr2++) {
         const auto a = _mm_loadu_si128 (ptr1);
         const auto b = _mm_loadu_si128 (ptr2);

         if (_mm_cmpistrc (a, b, mode)) {
            const auto idx = _mm_cmpistri (a, b, mode);

            const uint8_t b1 = (reinterpret_cast <char *> (ptr1))[idx];
            const uint8_t b2 = (reinterpret_cast <char *> (ptr2))[idx];

            if (b1 < b2) {
               return -1;
            }
            else if (b1 > b2) {
               return +1;
            }
            else {
               return 0;
            }
         }
         else if (_mm_cmpistrz (a, b, mode)) {
            break;
         }
      }
      return 0;
   }

   static int CR_SIMD_TARGET_AIL ("sse4.2") simd_sse42_strncmp (const char *s1, const char *s2, size_t n) {
      if (n == 0 || s1 == s2) {
         return 0;
      }
      auto ptr1 = reinterpret_cast<__m128i *> (const_cast <char *> (s1));
      auto ptr2 = reinterpret_cast<__m128i *> (const_cast <char *> (s2));

      constexpr uint8_t mode = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

      for (;; ptr1++, ptr2++) {
         const auto a = _mm_loadu_si128 (ptr1);
         const auto b = _mm_loadu_si128 (ptr2);

         if (_mm_cmpestrc (a, n, b, n, mode)) {
            const auto idx = _mm_cmpestri (a, n, b, n, mode);

            const uint8_t b1 = (reinterpret_cast<char *> (ptr1))[idx];
            const uint8_t b2 = (reinterpret_cast <char *> (ptr2))[idx];

            if (b1 < b2) {
               return -1;
            }
            else if (b1 > b2) {
               return +1;
            }
            else {
               return 0;
            }
         }
         else if (_mm_cmpestrz (a, n, b, n, mode)) {
            break;
         }
      }
      return 0;
   }
#endif

public:
   void init () {
      // this is not effective when builds are done with native optimizations
#if defined (CR_HAS_SIMD_SSE) && !defined (CR_NATIVE_BUILD)
      if (cpuflags.sse42) {
         strlen_ = reinterpret_cast <StrLen *> (simd_sse42_strlen);
         strcmp_ = reinterpret_cast <StrCmp *> (simd_sse42_strcmp);
         memcmp_ = reinterpret_cast <MemCmp *> (simd_sse42_memcmp);
         strncmp_ = reinterpret_cast <StrNCmp *> (simd_sse42_strncmp);
      }
#endif
   }
};

// expose simd-string singleton
CR_EXPOSE_GLOBAL_SINGLETON (SimdString, simdstring);

// declares libc function that replaced by crlib version
#define DECLARE_CRLIB_LIBC_FN(fn) \
   template <typename ...Args> static inline auto fn (Args &&...args) { \
      return SimdString::instance ().fn##_ (cr::forward <Args> (args)...); \
   } \

// declare our replacements
DECLARE_CRLIB_LIBC_FN (strlen);
DECLARE_CRLIB_LIBC_FN (memcmp);
DECLARE_CRLIB_LIBC_FN (strcmp);
DECLARE_CRLIB_LIBC_FN (strncmp);

CR_NAMESPACE_END
