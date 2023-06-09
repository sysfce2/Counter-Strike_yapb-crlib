//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
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
   template <bool AVOID_NAN = false> static inline CR_SSE_TARGET_AIL ("sse4.1") __m128 _mm_dot4_ps (__m128 v0, __m128 v1) {
      if (cpuflags.sse41) {
         if constexpr (AVOID_NAN) {
            return _mm_dp_ps (v0, v1, 0x7f);
         }
         return _mm_dp_ps (v0, v1, 0x71);
      }
      v0 = _mm_mul_ps (v0, v1);

      v1 = _mm_shuffle_ps (v0, v0, _MM_SHUFFLE (2, 3, 0, 1));
      v0 = _mm_add_ps (v0, v1);
      v1 = _mm_shuffle_ps (v0, v0, _MM_SHUFFLE (0, 1, 2, 3));
      v0 = _mm_add_ps (v0, v1);

      if constexpr (AVOID_NAN) {
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
   CR_SSE_TARGET ("sse4.1")
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
#if defined (CR_HAS_SSE)
   static inline size_t CR_SSE_TARGET_AIL ("sse4.2") sse42_strlen (const char *s) {
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

   static inline int CR_SSE_TARGET_AIL ("sse4.2") sse42_memcmp (const void *s1, const void *s2, size_t n) {
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

   static inline int CR_SSE_TARGET_AIL ("sse4.2") sse42_strcmp (const char *s1, const char *s2) {
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

   static inline int CR_SSE_TARGET_AIL ("sse4.2") sse42_strncmp (const char *s1, const char *s2, size_t n) {
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
#if defined (CR_HAS_SSE) && !defined (CR_NATIVE_BUILD)
      if (cpuflags.sse42 || plat.arm) {
         this->strlen = reinterpret_cast <StrLen *> (sse42_strlen);
         this->strcmp = reinterpret_cast <StrCmp *> (sse42_strcmp);
         this->memcmp = reinterpret_cast <MemCmp *> (sse42_memcmp);
         this->strncmp = reinterpret_cast <StrNCmp *> (sse42_strncmp);
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
