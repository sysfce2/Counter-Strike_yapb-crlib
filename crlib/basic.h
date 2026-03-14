// SPDX-License-Identifier: Unlicense

#pragma once

// our global namespace
#define CR_NAMESPACE_BEGIN namespace cr {
#define CR_NAMESPACE_END }

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <stddef.h>
#include <limits.h>

CR_NAMESPACE_BEGIN

// global helper stuff
template <typename T, size_t N> constexpr size_t bufsize (const T (&)[N]) {
   return N - 1;
}

template <typename T> constexpr T bit (const T &a) {
   return static_cast <T> (1ULL << a);
}

template <typename T> constexpr T min (const T &a, const T &b) {
   return a < b ? a : b;
}

template <typename T> constexpr T max (const T &a, const T &b) {
   return a > b ? a : b;
}

template <typename T> constexpr T clamp (const T &x, const T &a, const T &b) {
   return min (max (x, a), b);
}

constexpr size_t bit_ceil (size_t n) noexcept {
   if (n == 0) {
      return 1;
   }
   n--;

   n |= n >> 1;
   n |= n >> 2;
   n |= n >> 4;
   n |= n >> 8;
   n |= n >> 16;

#if defined(CR_ARCH_X64) || (SIZE_MAX > 0xffffffffu)
   n |= n >> 32;
#endif

   if (n == SIZE_MAX) {
      return n;
   }
   return n + 1;
}

// simple non-copying base class
class NonCopyable {
protected:
   explicit NonCopyable () = default;
   ~NonCopyable () = default;

public:
   NonCopyable (const NonCopyable &) = delete;
   NonCopyable &operator = (const NonCopyable &) = delete;
};

// simple non-movable base class
class NonMovable {
protected:
   explicit NonMovable () = default;
   ~NonMovable () = default;

public:
   NonMovable (NonMovable &&) = delete;
   NonMovable &operator = (NonMovable &&) = delete;
};

// singleton for objects
template <typename T> class Singleton : public NonCopyable, NonMovable {
protected:
   explicit Singleton () = default;
   ~Singleton () = default;

public:
   static inline T &instance () {
      static T __instance {};
      return __instance;
   }
};

// simple scoped enum, instead of enum class
#define CR_DECLARE_SCOPED_ENUM_TYPE(enumName, enumType, ...) \
   CR_NAMESPACE_BEGIN                                        \
   namespace enums {                                         \
      struct _##enumName : public NonCopyable, NonMovable {  \
         enum Type : enumType {                              \
            __VA_ARGS__                                      \
         };                                                  \
      };                                                     \
   }                                                         \
   CR_NAMESPACE_END                                          \
   using enumName = ::cr::enums::_##enumName::Type;          \

// same as above, but with int32_t type
#define CR_DECLARE_SCOPED_ENUM(enumName, ...)                   \
   CR_DECLARE_SCOPED_ENUM_TYPE(enumName, int32_t, __VA_ARGS__)   \

// exposes global variable from class singleton
#define CR_EXPOSE_GLOBAL_SINGLETON(className, globalVar)  \
   static inline auto &globalVar { className::instance () }      \

CR_NAMESPACE_END

// platform-dependant-stuff
#include <crlib/platform.h>
