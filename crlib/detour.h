// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/basic.h>

#if defined(CR_PSVITA) || defined(CR_ARCH_ARM) || defined(CR_ARCH_PPC) || defined(CR_ARCH_RISCV)

CR_NAMESPACE_BEGIN

template <typename T> class Detour final : public NonCopyable {
public:
   explicit Detour () = default;
   ~Detour () = default;

   Detour (StringRef module, StringRef name, T *address) {
      initialize (module, name, address);
   }

public:
   void initialize (StringRef, StringRef, T *) {}
   void install (void *, const bool = false) {}

   bool valid () const { return false; }
   bool detoured () const { return false; }
   bool detour () { return false; }
   bool restore () { return false; }

public:
   template <typename... Args> decltype (auto) operator () (Args &&...args) {
      T *fn = nullptr;
      return fn (cr::forward <Args> (args)...);
   }
};

CR_NAMESPACE_END

#else

#if !defined(CR_WINDOWS)
#  include <sys/mman.h>
#  include <pthread.h>
#endif

#include <crlib/thread.h>

CR_NAMESPACE_BEGIN

template <typename T> class Detour final : public NonCopyable {
private:
   enum : uint32_t {
      PtrSize = sizeof (void *),

#if defined(CR_ARCH_X64)
      JmpOffset = 2
#else
      JmpOffset = 1
#endif
   };

#if defined(CR_ARCH_X64)
   using uintptr = uint64_t;
#else
   using uintptr = uint32_t;
#endif

private:
   class ScopedRestore final {
   private:
      Detour <T> *detour_;

   public:
      explicit ScopedRestore (Detour *detour) : detour_ (detour) {
         detour_->restore ();
      }

      ~ScopedRestore () {
         detour_->detour ();
      }
   };

private:
   Mutex cs_;
   void *original_ { nullptr };
   void *detour_ { nullptr };
   Array <uint8_t> savedBytes_ {};
   bool overwritten_ { false };

#if defined(CR_ARCH_X64)
   Array <uint8_t> jmpBuffer_ { 0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0 };
#else
   Array <uint8_t> jmpBuffer_ { 0xb8, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0 };
#endif

#if !defined(CR_WINDOWS)
   unsigned long pageSize_ { 0 };
   uintptr pageStart_ { 0 };
#endif

public:
   explicit Detour () = default;

   Detour (StringRef module, StringRef name, T *address) {
      initialize (module, name, address);
   }

   ~Detour () {
      restore ();
      original_ = nullptr;
      detour_ = nullptr;
   }

public:
   void initialize (StringRef module, StringRef name, T *address) {
      savedBytes_.resize (jmpBuffer_.length ());

#if !defined(CR_WINDOWS)
      (void) module;
      (void) name;

      auto ptr = reinterpret_cast <uint8_t *> (address);

      while (*reinterpret_cast <uint16_t *> (ptr) == 0x25ff) {
         ptr = **reinterpret_cast <uint8_t ***> (ptr + 2);
      }
      original_ = ptr;
      pageSize_ = static_cast <unsigned long> (sysconf (_SC_PAGE_SIZE));
#else
      auto handle = GetModuleHandleA (module.chars ());

      if (!handle) {
         original_ = reinterpret_cast <void *> (address);
         return;
      }
      original_ = reinterpret_cast <void *> (GetProcAddress (handle, name.chars ()));

      if (!original_) {
         original_ = reinterpret_cast <void *> (address);
      }
#endif
   }

   void install (void *detour, const bool enable = false) {
      if (!original_) {
         return;
      }
      detour_ = detour;

#if !defined(CR_WINDOWS)
      pageStart_ = reinterpret_cast <uintptr> (original_) & ~(pageSize_ - 1);
#endif

      memcpy (savedBytes_.data (), original_, savedBytes_.length ());
      memcpy (reinterpret_cast <void *> (reinterpret_cast <uintptr> (jmpBuffer_.data ()) + JmpOffset), &detour_, PtrSize);

      if (enable) {
         this->detour ();
      }
   }

   bool valid () const {
      return original_ != nullptr && detour_ != nullptr;
   }

   bool detoured () const {
      return overwritten_;
   }

   bool detour () {
      if (!valid ()) {
         return false;
      }
      return overwriteMemory (jmpBuffer_, true);
   }

   bool restore () {
      if (!valid ()) {
         return false;
      }
      return overwriteMemory (savedBytes_, false);
   }

   template <typename... Args> decltype (auto) operator () (Args &&...args) {
      ScopedRestore sw { this };
      return reinterpret_cast <T *> (original_) (cr::forward <Args> (args)...);
   }

private:
   bool overwriteMemory (const Array <uint8_t> &to, const bool overwritten) noexcept {
      MutexScopedLock lock (cs_);
      overwritten_ = overwritten;

#if defined(CR_WINDOWS)
      unsigned long oldProtect {};

      if (!VirtualProtect (original_, to.length (), PAGE_EXECUTE_READWRITE, &oldProtect)) {
         return false;
      }
      memcpy (original_, to.data (), to.length ());
      return VirtualProtect (original_, to.length (), oldProtect, &oldProtect) != 0;
#else
      if (mprotect (reinterpret_cast <void *> (pageStart_), pageSize_, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
         return false;
      }
      memcpy (original_, to.data (), to.length ());
      return mprotect (reinterpret_cast <void *> (pageStart_), pageSize_, PROT_READ | PROT_EXEC) != -1;
#endif
   }
};

CR_NAMESPACE_END

#endif
