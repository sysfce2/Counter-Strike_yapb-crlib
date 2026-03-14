// SPDX-License-Identifier: Unlicense

#pragma once

#if !defined(CR_ARCH_NON_X86)
#  if defined(CR_WINDOWS) && defined(CR_CXX_MSVC)
#     include <intrin.h>
#  elif !defined(CR_EMSCRIPTEN)
#     include <cpuid.h>
#  endif
#elif (defined(CR_LINUX) || defined(CR_ANDROID)) && (defined(CR_ARCH_ARM) || defined(CR_ARCH_RISCV))
#  include <sys/auxv.h>
#endif

CR_NAMESPACE_BEGIN

// cpu flags for current cpu
class CpuFlags final : public Singleton <CpuFlags> {
public:
   // x86 simd
   bool sse3 {}, ssse3 {}, sse41 {}, sse42 {};
   bool avx {}, avx2 {}, avx512f {};
   bool fma {}, f16c {}, popcnt {};
   bool bmi1 {}, bmi2 {};
   bool aesni {}, shani {};

   // arm
   bool neon {}, fp16 {}, dotprod {};
   bool sve {}, sve2 {};
   bool bf16 {}, i8mm {};
   bool aes {}, sha1 {}, sha2 {};
   bool pmull {}, crc32 {}, atomics {};

   // risc-v
   bool rvv {};

public:
   CpuFlags () {
      detect ();
   }

   ~CpuFlags () = default;

private:
   void detect () {
#if !defined(CR_ARCH_NON_X86) && !defined(CR_EMSCRIPTEN)
      detectX86 ();
#elif defined(CR_ARCH_ARM)
      detectArm ();
#elif defined(CR_ARCH_RISCV)
      detectRiscV ();
#endif
   }

#if !defined(CR_ARCH_NON_X86) && !defined(CR_EMSCRIPTEN)
   void detectX86 () {
      enum : int32_t { eax, ebx, ecx, edx, count };
      uint32_t data[count] {};

      // leaf 1
#if defined(CR_WINDOWS) && defined(CR_CXX_MSVC)
      __cpuidex (reinterpret_cast <int32_t *> (data), 1, 0);
#else
      __get_cpuid (0x1, &data[eax], &data[ebx], &data[ecx], &data[edx]);
#endif

      sse3   = !!(data[ecx] & cr::bit (0));
      ssse3  = !!(data[ecx] & cr::bit (9));
      fma    = !!(data[ecx] & cr::bit (12));
      sse41  = !!(data[ecx] & cr::bit (19));
      sse42  = !!(data[ecx] & cr::bit (20));
      popcnt = !!(data[ecx] & cr::bit (23));
      aesni  = !!(data[ecx] & cr::bit (25));
      avx    = !!(data[ecx] & cr::bit (28));
      f16c   = !!(data[ecx] & cr::bit (29));

      // leaf 7, sub-leaf 0
#if defined(CR_WINDOWS) && defined(CR_CXX_MSVC)
      __cpuidex (reinterpret_cast <int32_t *> (data), 7, 0);
#else
      __get_cpuid (0x7, &data[eax], &data[ebx], &data[ecx], &data[edx]);
#endif

      bmi1    = !!(data[ebx] & cr::bit (3));
      avx2    = !!(data[ebx] & cr::bit (5));
      bmi2    = !!(data[ebx] & cr::bit (8));
      avx512f = !!(data[ebx] & cr::bit (16));
      shani   = !!(data[ebx] & cr::bit (29));
   }
#endif

#if defined(CR_ARCH_ARM)
   void detectArm () {
#if defined(CR_ARCH_ARM64)
      detectArm64 ();
#elif defined(CR_ARCH_ARM32)
      detectArm32 ();
#endif
   }

#if defined(CR_ARCH_ARM64)
   void detectArm64 () {
      neon = true;

#if defined(CR_LINUX) || defined(CR_ANDROID)
      auto hwcap = getauxval (AT_HWCAP);
      auto hwcap2 = getauxval (AT_HWCAP2);

      aes     = !!(hwcap & cr::bit (3));
      pmull   = !!(hwcap & cr::bit (4));
      sha1    = !!(hwcap & cr::bit (5));
      sha2    = !!(hwcap & cr::bit (6));
      crc32   = !!(hwcap & cr::bit (7));
      atomics = !!(hwcap & cr::bit (8));
      fp16    = !!(hwcap & cr::bit (10));
      dotprod = !!(hwcap & cr::bit (20));
      sve     = !!(hwcap & cr::bit (22));

      sve2 = !!(hwcap2 & cr::bit (1));
      i8mm = !!(hwcap2 & cr::bit (13));
      bf16 = !!(hwcap2 & cr::bit (14));
#else
      // compile-time detection for non-linux platforms (macos, etc.)
#if defined(__ARM_FEATURE_CRC32)
      crc32 = true;
#endif
#if defined(__ARM_FEATURE_AES) || defined(__ARM_FEATURE_CRYPTO)
      aes = true;
      pmull = true;
#endif
#if defined(__ARM_FEATURE_SHA2) || defined(__ARM_FEATURE_CRYPTO)
      sha1 = true;
      sha2 = true;
#endif
#if defined(__ARM_FEATURE_ATOMICS)
      atomics = true;
#endif
#if defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)
      fp16 = true;
#endif
#if defined(__ARM_FEATURE_DOTPROD)
      dotprod = true;
#endif
#if defined(__ARM_FEATURE_SVE)
      sve = true;
#endif
#if defined(__ARM_FEATURE_SVE2)
      sve2 = true;
#endif
#if defined(__ARM_FEATURE_BF16)
      bf16 = true;
#endif
#if defined(__ARM_FEATURE_MATMUL_INT8)
      i8mm = true;
#endif
#endif
   }
#endif

#if defined(CR_ARCH_ARM32)
   void detectArm32 () {
#if defined(CR_LINUX) || defined(CR_ANDROID)
      auto hwcap = getauxval (AT_HWCAP);
      auto hwcap2 = getauxval (AT_HWCAP2);

      neon = !!(hwcap & cr::bit (12));

      aes   = !!(hwcap2 & cr::bit (0));
      pmull = !!(hwcap2 & cr::bit (1));
      sha1  = !!(hwcap2 & cr::bit (2));
      sha2  = !!(hwcap2 & cr::bit (3));
      crc32 = !!(hwcap2 & cr::bit (4));
#else
#if defined(__ARM_NEON)
      neon = true;
#endif
#if defined(__ARM_FEATURE_CRC32)
      crc32 = true;
#endif
#if defined(__ARM_FEATURE_AES) || defined(__ARM_FEATURE_CRYPTO)
      aes = true;
      pmull = true;
#endif
#if defined(__ARM_FEATURE_SHA2) || defined(__ARM_FEATURE_CRYPTO)
      sha1 = true;
      sha2 = true;
#endif
#endif
   }
#endif
#endif

#if defined(CR_ARCH_RISCV)
   void detectRiscV () {
#if defined(CR_LINUX)
      enum : uint32_t { hwcapV = cr::bit ('V' - 'A') };
      rvv = !!(getauxval (AT_HWCAP) & hwcapV);
#elif defined(__riscv_vector)
      rvv = true;
#endif
   }
#endif
};

// expose platform singleton
CR_EXPOSE_GLOBAL_SINGLETON (CpuFlags, cpuflags);

CR_NAMESPACE_END
