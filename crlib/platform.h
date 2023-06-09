//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

CR_NAMESPACE_BEGIN

// detects the build platform
#if defined(__linux__)
#  define CR_LINUX
#elif defined(__APPLE__)
#  define CR_OSX
#elif defined(_WIN32)
#  define CR_WINDOWS
#endif

#if defined(__ANDROID__)
#  define CR_ANDROID
#     if defined(LOAD_HARDFP)
#        define CR_ANDROID_HARD_FP
#     endif
#endif

#if !defined (CR_DEBUG) && (defined(DEBUG) || defined(_DEBUG))
#  define CR_DEBUG
#endif

// detects the compiler
#if defined(_MSC_VER)
#  define CR_CXX_MSVC _MSC_VER
#endif

#if defined(__clang__)
#  define CR_CXX_CLANG __clang__
#endif

#if defined(__GNUC__)
#  define CR_CXX_GCC __GNUC__
#endif

// configure macroses
#define CR_LINKAGE_C extern "C"

#if defined(CR_WINDOWS)
#  define CR_EXPORT CR_LINKAGE_C __declspec (dllexport)
#  define CR_STDCALL __stdcall
#elif defined(CR_LINUX) || defined(CR_OSX)
#  define CR_EXPORT CR_LINKAGE_C __attribute__((visibility("default")))
#  define CR_STDCALL
#else
#  error "Can't configure export macros. Compiler unrecognized."
#endif

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined (__aarch64__) || (defined(_MSC_VER) && defined(_M_X64))
#  define CR_ARCH_X64
#elif defined(__i686) || defined(__i686__) || defined(__i386) || defined(__i386__) || defined(i386) || (defined(_MSC_VER) && defined(_M_IX86))
#  define CR_ARCH_X86
#endif

#if defined(__arm__)
#  define CR_ARCH_ARM32
#elif defined(__aarch64__)
#  define CR_ARCH_ARM64
#endif

#if defined (CR_ARCH_ARM32) || defined (CR_ARCH_ARM64)
#   define CR_ARCH_ARM
#endif


#if !defined(CR_DISABLE_SSE)
#  define CR_HAS_SSE
#  if !defined (CR_ARCH_ARM)
#     define CR_INTRIN_INCLUDE <smmintrin.h>
#  else
#     define CR_INTRIN_INCLUDE <crlib/ssemath/sse2neon.h>
#  endif
#endif

#if defined(CR_HAS_SSE)
#  if defined (CR_CXX_MSVC)
#     define CR_ALIGN16 __declspec (align (16))
#  else
#     define CR_ALIGN16 __attribute__((aligned(16)))
#  endif
#endif

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define CR_ARCH_CPU_BIG_ENDIAN
#endif

// msvc provides us placement new by default
#if defined (CR_CXX_MSVC)
#  define __PLACEMENT_NEW_INLINE
#endif

#if (defined (CR_CXX_MSVC) && !defined (CR_CXX_CLANG)) || defined (CR_ARCH_ARM)
#  define CR_SSE_TARGET(dest)
#  define CR_SSE_TARGET_AIL(dest)
#else
#  define CR_SSE_TARGET(dest) __attribute__((target(dest)))
#  define CR_SSE_TARGET_AIL(dest) __attribute__((__always_inline__, target(dest)))
#endif

// ship windows xp release builds only with msvc
#if defined (CR_CXX_MSVC) && !defined(CR_DEBUG)
#  define CR_HAS_WINXP_SUPPORT
#endif

// set the minimal glibc as we can
#if defined (CR_ARCH_ARM)
#  define GLIBC_VERSION_MIN "2.17"
#elif defined (CR_ARCH_X64) && !defined (CR_ARCH_ARM)
#  define GLIBC_VERSION_MIN "2.2.5"
#else
#  define GLIBC_VERSION_MIN "2.0"
#endif

// avoid linking to high GLIBC versions
#if defined (CR_LINUX)
   __asm__ (".symver dlsym, dlsym@GLIBC_" GLIBC_VERSION_MIN);
   __asm__ (".symver dladdr, dladdr@GLIBC_" GLIBC_VERSION_MIN);
   __asm__ (".symver dlclose, dlclose@GLIBC_" GLIBC_VERSION_MIN);
   __asm__ (".symver dlopen, dlopen@GLIBC_" GLIBC_VERSION_MIN);
#endif

CR_NAMESPACE_END

#if defined(CR_WINDOWS)
constexpr auto PATH_SEP = "\\";
constexpr auto DLL_SUFFIX = ".dll";

// raise windows api version if doesn't build for xp
#if !defined(CR_HAS_WINXP_SUPPORT) && !defined (CR_CXX_MSVC)
#  define _WIN32_WINNT 0x0600
#  define WINVER 0x0600 
#endif

#  define WIN32_LEAN_AND_MEAN
#  define NOGDICAPMASKS
#  define NOVIRTUALKEYCODES
#  define NOWINMESSAGES
#  define NOWINSTYLES
#  define NOSYSMETRICS
#  define NOMENUS
#  define NOICONS
#  define NOKEYSTATES
#  define NOSYSCOMMANDS
#  define NORASTEROPS
#  define NOSHOWWINDOW
#  define OEMRESOURCE
#  define NOATOM
#  define NOCLIPBOARD
#  define NOCOLOR
#  define NOCTLMGR
#  define NODRAWTEXT
#  define NOGDI
#  define NOKERNEL
#  define NONLS
#  define NOMEMMGR
#  define NOMETAFILE
#  define NOMINMAX
#  define NOMSG
#  define NOOPENFILE
#  define NOSCROLL
#  define NOSERVICE
#  define NOSOUND
#  define NOTEXTMETRIC
#  define NOWH
#  define NOWINOFFSETS
#  define NOCOMM
#  define NOKANJI
#  define NOHELP
#  define NOPROFILER
#  define NODEFERWINDOWPOS
#  define NOMCX
#  define NOWINRES
#  define NOIME

#  include <windows.h>
#  include <direct.h>
#  include <io.h>
#else
constexpr auto PATH_SEP = "/";
#  if defined (CR_OSX)
      constexpr auto DLL_SUFFIX = ".dylib";
#  else
      constexpr auto DLL_SUFFIX = ".so";
#endif
#  include <unistd.h>
#  include <strings.h>
#  include <sys/time.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <locale.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>

#if defined (CR_ANDROID)
#  include <android/log.h>
#endif

#include <time.h>

CR_NAMESPACE_BEGIN

// helper struct for platform detection
struct Platform : public Singleton <Platform> {
   bool win = false;
   bool nix = false;
   bool osx = false;
   bool android = false;
   bool hfp = false;
   bool x64 = false;
   bool arm = false;

   char appName[64] = {};

   Platform () {
#if defined(CR_WINDOWS)
      win = true;
#endif

#if defined(CR_ANDROID)
      android = true;

#  if defined (CR_ANDROID_HARD_FP)
      hfp = true;
#  endif
#endif

#if defined(CR_LINUX)
      nix = true;
#endif

#if defined(CR_OSX)
      osx = true;
#endif

#if defined(CR_ARCH_X64) || defined(CR_ARCH_ARM64)
      x64 = true;
#endif

#if defined(CR_ARCH_ARM)
      arm = true;
#endif
   }

   // set the app name
   void setAppName (const char *name) {
      snprintf (appName, cr::bufsize (appName), "%s", name);
   }

   // helper platform-dependant functions
   template <typename U> bool checkPointer (U *ptr) {
#if defined(CR_WINDOWS)
      if (IsBadCodePtr (reinterpret_cast <FARPROC> (ptr))) {
         return false;
      }
#else
      (void) (ptr);
#endif
      return true;
   }

   bool createDirectory (const char *dir) {
      int result = 1;
#if defined(CR_WINDOWS)
      result = _mkdir (dir);
#else
      result = mkdir (dir, 0777);
#endif
      return result == 0;
   }

   bool removeFile (const char *dir) {
#if defined(CR_WINDOWS)
      _unlink (dir);
#else
      unlink (dir);
#endif
      return true;
   }

   bool hasModule (const char *mod) {
#if defined(CR_WINDOWS)
      return GetModuleHandleA (mod) != nullptr;
#else
      (void) (mod);
      return true;
#endif
   }

   float seconds () {
#if defined(CR_WINDOWS)
      LARGE_INTEGER count {}, freq {};

      count.QuadPart = 0;
      freq.QuadPart = 0;

      QueryPerformanceFrequency (&freq);
      QueryPerformanceCounter (&count);

      return static_cast <float> (count.QuadPart / freq.QuadPart);
#else
      timeval tv;
      gettimeofday (&tv, nullptr);

      static auto startTime = tv.tv_sec;

      return static_cast <float> (tv.tv_sec - startTime);
#endif
   }

   void abort (const char *msg = "OUT OF MEMORY!") noexcept {
      fprintf (stderr, "%s\n", msg);

#if defined (CR_ANDROID)
      __android_log_write (ANDROID_LOG_ERROR, appName, msg);
#endif

#if defined(CR_WINDOWS)
      DestroyWindow (GetForegroundWindow ());
      MessageBoxA (GetActiveWindow (), msg, appName, MB_ICONSTOP);
#endif

#if defined(CR_DEBUG) && defined(CR_CXX_MSVC)
      DebugBreak ();
#else
      ::abort ();
#endif
   }

   // anologue of memset
   template <typename U> void bzero (U *ptr, size_t len) noexcept {
      memset (reinterpret_cast <void *> (ptr), 0, len);
   }

   void loctime (tm *_tm, const time_t *_time) {
#if defined (CR_WINDOWS)
      localtime_s (_tm, _time);
#else
      localtime_r (_time, _tm);
#endif
   }

   const char *env (const char *var) {
      static char result[256];
      bzero (result, sizeof (result));

#if defined(CR_CXX_MSVC)
      char *buffer = nullptr;
      size_t size = 0;

      if (_dupenv_s (&buffer, &size, var) == 0 && buffer != nullptr) {
         strncpy_s (result, buffer, sizeof (result));
         free (buffer);
      }
#else
      auto data = getenv (var);

      if (data) {
         strncpy (result, data, cr::bufsize (result));
      }
#endif
      return result;
   }

   int32_t hardwareConcurrency () {
#if defined (CR_WINDOWS)
      SYSTEM_INFO sysinfo;
      GetSystemInfo (&sysinfo);

      return sysinfo.dwNumberOfProcessors;
#else
      return sysconf (_SC_NPROCESSORS_ONLN);
#endif
   }

   bool fileExists (const char *path) {
#if defined (CR_WINDOWS)
      return _access (path, 0) == 0;
#else
      return access (path, F_OK) == 0;
#endif
   }

   FILE *openStdioFile (const char *path, const char *mode) {
      FILE *handle = nullptr;

#if defined (CR_CXX_MSVC)
      fopen_s (&handle, path, mode);
#else
      handle = fopen (path, mode);
#endif
      return handle;
   }
};

// expose platform singleton
CR_EXPOSE_GLOBAL_SINGLETON (Platform, plat);

CR_NAMESPACE_END
