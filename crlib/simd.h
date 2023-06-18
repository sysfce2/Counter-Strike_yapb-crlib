//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>

#if defined (CR_HAS_SIMD)
#  include CR_INTRIN_INCLUDE
#  if defined (CR_ARCH_ARM)
#     include <crlib/ssemath/sincos_arm.h>
#endif
#endif

CR_NAMESPACE_BEGIN

#if defined (CR_HAS_SIMD)

namespace ssemath {
#  include <crlib/ssemath/ssemath.h>
}

CR_SIMD_ALIGNED const auto simd_EPS = _mm_set1_ps (kFloatEpsilon);

// simple wrapper for vector
class CR_SIMD_ALIGNED SimdVec3Wrap final {
private:
   template <bool AVOID_NAN = false> static inline CR_SIMD_TARGET_AIL ("sse4.1") __m128 _mm_dot4_ps (__m128 v0, __m128 v1) {
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

public:
#if defined (CR_CXX_MSVC)
#   pragma warning(push)
#   pragma warning(disable: 4201)
#endif
   union {
      __m128 m { _mm_setzero_ps () };

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
   CR_SIMD_TARGET ("sse4.1")
   SimdVec3Wrap normalize () const {
#if defined(CR_ARCH_ARM)
      return { _mm_div_ps (m, _mm_sqrt_ps (_mm_dp_ps (m, m, 0x7f))) };
#else
      return { _mm_div_ps (m, _mm_sqrt_ps (_mm_dot4_ps <true> (m, m))) };
#endif
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

#if defined (CR_ARCH_ARM)
   void angleVectors (SimdVec3Wrap &sines, SimdVec3Wrap &cosines) {
      static constexpr CR_SIMD_ALIGNED float d2r[] = {
         kDegreeToRadians, kDegreeToRadians,
         kDegreeToRadians, kDegreeToRadians
      };
      neon_sincos_ps (_mm_mul_ps (m, _mm_load_ps (d2r)), sines.m, cosines.m);
   }
#else
   // this function directly taken from rehlds project https://github.com/dreamstalker/rehlds
   void angleVectors (float *forward, float *right, float *upward) {
      static constexpr CR_SIMD_ALIGNED float d2r[] = {
         kDegreeToRadians, kDegreeToRadians,
         kDegreeToRadians, kDegreeToRadians
      };

      static constexpr CR_SIMD_ALIGNED uint32_t negmask[4] = {
         0x80000000, 0x80000000, 0x80000000, 0x80000000
      };

      static constexpr CR_SIMD_ALIGNED uint32_t negmask_1001[4] = {
         0x80000000, 0, 0, 0x80000000
      };

      __m128 s, c;
      ssemath::sincos_ps (_mm_mul_ps (m, _mm_load_ps (d2r)), s, c);

      auto m1 = _mm_shuffle_ps (c, s, 0x90); // [cp][cp][sy][sr]
      auto m2 = _mm_shuffle_ps (c, c, 0x09); // [cy][cr][cp][cp]
      auto cp_mults = _mm_mul_ps (m1, m2); // [cp * cy][cp * cr][cp * sy][cp * sr];

      m1 = _mm_shuffle_ps (c, s, 0x15); // [cy][cy][sy][sp]
      m2 = _mm_shuffle_ps (s, c, 0xa0); // [sp][sp][cr][cr]
      m1 = _mm_shuffle_ps (m1, m1, 0xc8); // [cy][sy][cy][sp]

      auto m3 = _mm_shuffle_ps (s, s, 0x4a); // [sr][sr][sp][sy];
      m3 = _mm_mul_ps (m3, _mm_mul_ps (m1, m2)); // [sp*cy*sr][sp*sy*sr][cr*cy*sp][cr*sp*sy]

      m2 = _mm_shuffle_ps (s, c, 0x65); // [sy][sy][cr][cy]
      m1 = _mm_shuffle_ps (c, s, 0xa6); // [cr][cy][sr][sr]
      m2 = _mm_shuffle_ps (m2, m2, 0xd8); // [sy][cr][sy][cy]
      m1 = _mm_xor_ps (m1, _mm_load_ps (const_cast <float *> (reinterpret_cast <const float *> (&negmask_1001)))); // [-cr][cy][sr][-sr]
      m1 = _mm_mul_ps (m1, m2); // [-cr*sy][cy*cr][sr*sy][-sr*cy]

      m3 = _mm_add_ps (m3, m1);

      if (forward) {
         _mm_storel_pi (reinterpret_cast <__m64 *> (forward), _mm_shuffle_ps (cp_mults, cp_mults, 0x08));
         forward[2] = -_mm_cvtss_f32 (s);
      }

      if (right) {
         auto r = _mm_shuffle_ps (m3, cp_mults, 0xF4); // [m3(0)][m3(1)][cp(3)][cp(3)]
         r = _mm_xor_ps (r, _mm_load_ps (const_cast <float *> (reinterpret_cast <const float *> (&negmask))));

         _mm_storel_pi (reinterpret_cast <__m64 *> (right), r);
         _mm_store_ss (right + 2, _mm_shuffle_ps (r, r, 0x02));
      }

      if (upward) {
         _mm_storel_pi (reinterpret_cast <__m64 *> (upward), _mm_shuffle_ps (m3, m3, 0x0e));
         upward[2] = _mm_cvtss_f32 (_mm_shuffle_ps (cp_mults, cp_mults, 0x01));
      }
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
   StrLen *strlen { ::strlen };
   StrCmp *strcmp { ::strcmp };
   MemCmp *memcmp { ::memcmp };
   StrNCmp *strncmp { ::strncmp };

private:
#if defined (CR_HAS_SIMD)
   static inline size_t CR_SIMD_TARGET_AIL ("sse4.2") simd_sse42_strlen (const char *s) {
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

   static inline int CR_SIMD_TARGET_AIL ("sse4.2") simd_sse42_memcmp (const void *s1, const void *s2, size_t n) {
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

   static inline int CR_SIMD_TARGET_AIL ("sse4.2") simd_sse42_strcmp (const char *s1, const char *s2) {
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

   static inline int CR_SIMD_TARGET_AIL ("sse4.2") simd_sse42_strncmp (const char *s1, const char *s2, size_t n) {
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
#if defined (CR_HAS_SIMD) && !defined (CR_NATIVE_BUILD)
      if (cpuflags.sse42 || plat.arm) {
         this->strlen = reinterpret_cast <StrLen *> (simd_sse42_strlen);
         this->strcmp = reinterpret_cast <StrCmp *> (simd_sse42_strcmp);
         this->memcmp = reinterpret_cast <MemCmp *> (simd_sse42_memcmp);
         this->strncmp = reinterpret_cast <StrNCmp *> (simd_sse42_strncmp);
      }
#endif
   }
};

// expose simd-string singleton
CR_EXPOSE_GLOBAL_SINGLETON (SimdString, simdstring);

// declares libc function that replaced by crlib version
#define DECLARE_CRLIB_LIBC_FN(fn) \
   template <typename ...Args> static inline auto fn (Args &&...args) { \
      return SimdString::instance ().fn (cr::forward <Args> (args)...); \
   } \

// declare our replacements
DECLARE_CRLIB_LIBC_FN (strlen);
DECLARE_CRLIB_LIBC_FN (memcmp);
DECLARE_CRLIB_LIBC_FN (strcmp);
DECLARE_CRLIB_LIBC_FN (strncmp);

CR_NAMESPACE_END
