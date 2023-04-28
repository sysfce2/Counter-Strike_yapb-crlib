//
// YaPB - Counter-Strike Bot based on PODBot by Markus Klinge.
// Copyright © 2004-2022 YaPB Project <yapb@jeefo.net>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>

#define _USE_MATH_DEFINES
#include <math.h>

CR_NAMESPACE_BEGIN

constexpr float kFloatOnEpsilon = 0.01f;
constexpr float kFloatEqualEpsilon = 0.001f;
constexpr float kFloatEpsilon = 1.192092896e-07f;
constexpr float kMathPi = 3.141592653583f;
constexpr float kDegreeToRadians = kMathPi / 180.0f;
constexpr float kRadiansToDegree = 180.0f / kMathPi;

CR_NAMESPACE_END

#include <crlib/simd.h>

CR_NAMESPACE_BEGIN

#if defined (CR_HAS_SSE)
template <> inline float abs (const float &x) {
   return _mm_cvtss_f32 (ssemath::fabs_ps (_mm_load_ss (&x)));
}

template <> inline float min (const float &a, const float &b) {
   return _mm_cvtss_f32 (_mm_min_ss (_mm_load_ss (&a), _mm_load_ss (&b)));
}

template <> inline float max (const float &a, const float &b) {
   return _mm_cvtss_f32 (_mm_max_ss (_mm_load_ss (&a), _mm_load_ss (&b)));
}

template <> inline float clamp (const float &x, const float &a, const float &b) {
   return _mm_cvtss_f32 (_mm_min_ss (_mm_max_ss (_mm_load_ss (&x), _mm_load_ss (&a)), _mm_load_ss (&b)));
}
#endif

inline float sinf (const float value) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (ssemath::sin_ps (_mm_load_ss (&value)));
#else
   return ::sinf (value);
#endif
}

inline float cosf (const float value) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (ssemath::cos_ps (_mm_load_ss (&value)));
#else
   return ::cosf (value);
#endif
}

inline float atan2f (const float y, const float x) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (ssemath::atan2_ps (_mm_load_ss (&y), _mm_load_ss (&x)));
#else
   return ::atan2f (y, x);
#endif
}

inline float powf (const float x, const float y) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (ssemath::pow_ps (_mm_load_ss (&x), _mm_load_ss (&y)));
#else
   return ::powf (x, y);
#endif
}

inline float sqrtf (const float value) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (_mm_sqrt_ss (_mm_load_ss (&value)));
#else
   return ::sqrtf (value);
#endif
}

inline float rsqrtf (const float value) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (_mm_rsqrt_ss (_mm_load_ss (&value)));
#else
   return 1.0f / ::sqrtf (value);
#endif
}

inline float tanf (const float value) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (ssemath::tan_ps (_mm_load_ss (&value)));
#else
   return ::tanf (value);
#endif
}

inline float ceilf (const float value) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (ssemath::ceil_ps (_mm_load_ss (&value)));
#else
   return ::ceilf (value);
#endif
}

inline float log10 (const float value) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (ssemath::log10_ps (_mm_load_ss (&value)));
#else
   return ::log10f (value);
#endif
}

inline float floorf (const float value) {
#if defined (CR_HAS_SSE)
   return _mm_cvtss_f32 (cr::ssemath::floor_ps (_mm_load_ss (&value)));
#else
   return ::floorf (value);
#endif
}

static inline void sincosf (const float &x, float &s, float &c) {
#if defined (CR_HAS_SSE)
   __m128 _s, _c;
   ssemath::sincos_ps (_mm_load_ss (&x), _s, _c);

   s = _mm_cvtss_f32 (_s);
   c = _mm_cvtss_f32 (_c);
#else
   c = cr::cosf (x);
   s = cr::sqrtf (static_cast <float> (1) - cr::sqrf (c));

   if (static_cast <int> (x / cr::kMathPi) & 1) {
      s = -s;
   }
#endif
}

static inline bool fzero (const float e) {
   return cr::abs (e) < kFloatOnEpsilon;
}

static inline bool fequal (const float a, const float b) {
   return cr::abs (a - b) < kFloatEqualEpsilon;
}

constexpr float rad2deg (const float r) {
   return r * kRadiansToDegree;
}

constexpr float deg2rad (const float d) {
   return d * kDegreeToRadians;
}

template <int Degree> float _wrapAngleFn (float x) {
   return x - 2.0f * static_cast <float> (Degree) * cr::floorf (x / (2.0f * static_cast <float> (Degree)) + 0.5f);
}

static inline float wrapAngle360 (float a) {
   return _wrapAngleFn <360> (a);
}

static inline float wrapAngle (const float a) {
   return _wrapAngleFn <180> (a);
}

static inline float anglesDifference (const float a, const float b) {
   return wrapAngle (a - b);
}

// approximation functions
namespace apx {
   inline float sqrtf (const float value) {
#if defined (CR_ARCH_ARM)
      return cr::sqrtf (value);
#else
      union {
         float f;
         uint32_t u;
      } val { value };

      val.u = (val.u >> 1U) + 0x1fbb4000;
      return val.f;
#endif
   }

   inline float rsqrtf (const float value) {
#if defined (CR_ARCH_ARM)
      return cr::rsqrtf (value);
#else
      union {
         float f;
         uint32_t u;
      } val = { value };
      val.u = 0x5f1ffff9 - (val.u >> 1U);
      val.f *= 0.703952253f * (2.38924456f - value * cr::sqrf (val.f));
      return val.f;
#endif
   }
}


CR_NAMESPACE_END
