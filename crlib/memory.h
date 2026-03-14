// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/basic.h>
#include <crlib/movable.h>
#include <crlib/platform.h>

// provide placement new to avoid stdc++ <new> header
#if !defined(CR_COMPAT_STL)
inline void *operator new (const size_t, void *ptr) noexcept {
   return ptr;
}
#endif

CR_NAMESPACE_BEGIN

// internal memory manager
namespace mem {

   // allocates raw memory for length objects of type T
   template <typename T> CR_FORCE_INLINE T *allocate (const size_t length = 1) noexcept {
      auto size = length * sizeof (T);
      auto memory = reinterpret_cast <T *> (malloc (size));

      if (!memory) {
         char errmsg[384] {};
         snprintf (errmsg, sizeof (errmsg), "Failed to allocate %zd kbytes of memory. Closing down.", size / 1024);

         plat.abort (errmsg);
      }
      return memory;
   }

   // releases memory and returns nullptr
   template <typename T> CR_FORCE_INLINE T *release (T *memory) noexcept {
      free (memory);
      return nullptr;
   }

   // in-place constructs a single object with forwarded arguments
   template <typename T, typename ...Args> CR_FORCE_INLINE T *construct (T *memory, Args &&...args) noexcept {
      new (memory) T (cr::forward <Args> (args)...);
      return memory;
   }

   // calls destructor on a single object
   template <typename T> CR_FORCE_INLINE void destruct (T *memory) noexcept {
      memory->~T ();
   }

   // in-place constructs an array of objects with forwarded arguments
   template <typename T, typename ...Args> CR_FORCE_INLINE T *constructArray (T *memory, size_t length, Args &&...args) noexcept {
      for (size_t i = 0; i < length; ++i) {
         new (&memory[i]) T (cr::forward <Args> (args)...);
      }
      return memory;
   }

   // calls destructors on an array of objects
   template <typename T> CR_FORCE_INLINE void destructArray (T *memory, size_t length) noexcept {
      for (size_t i = 0; i < length; ++i) {
         memory[i].~T ();
      }
   }

   // allocates and constructs a single object
   template <typename T, typename ...Args> CR_FORCE_INLINE T *allocateAndConstruct (Args &&...args) noexcept {
      return construct <T> (allocate <T> (), cr::forward <Args> (args)...);
   }

   // moves elements from src to dest, destroying src elements
   template <typename T> CR_FORCE_INLINE void transfer (T *dest, T *src, size_t length) noexcept {
      if constexpr (cr::is_trivially_copyable_v <T>) {
         memcpy (dest, src, length * sizeof (T));
      }
      else {
         for (size_t i = 0; i < length; ++i) {
            construct <T> (&dest[i], cr::move (src[i]));
            destruct <T> (&src[i]);
         }
      }
   }
}

CR_NAMESPACE_END
