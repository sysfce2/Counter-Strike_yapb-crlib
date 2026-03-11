//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>
#include <crlib/movable.h>
#include <crlib/platform.h>

// provide placment new to avoid stdc++ <new> header
#if !defined(CR_COMPAT_STL)
inline void *operator new (const size_t, void *ptr) noexcept {
   return ptr;
}
#endif

CR_NAMESPACE_BEGIN

// internal memory manager
class Memory final {
public:
   constexpr Memory () = default;
   ~Memory () = default;

public:
   template <typename T> static T *get (const size_t length = 1) noexcept {
       auto size = cr::max <size_t> (size_t (1), length) * sizeof (T);

#if defined(CR_CXX_GCC)
      if (size >= PTRDIFF_MAX) {
         plat.abort ();
      }
#endif
      auto memory = reinterpret_cast <T *> (malloc (size));

      if (!memory) {
         char errmsg[384] {};
         snprintf (errmsg, cr::bufsize (errmsg), "Failed to allocate %zd kbytes of memory. Closing down.", size / 1024);

         plat.abort (errmsg);
      }
      return memory;
   }

   template <typename T> static T *release (T *memory) noexcept {
       free (memory);
       return nullptr;
    }

public:
   template <typename T, typename ...Args> static T *construct (T *memory, Args &&...args) noexcept {
      new (memory) T (cr::forward <Args> (args)...);
      return memory;
   }

   template <typename T> static void destruct (T *memory) {
       memory->~T ();
    }

   template <typename T, typename ...Args> static T *constructArray (T *memory, size_t length, Args &&...args) noexcept {
       for (size_t i = 0; i < length; ++i) {
          new (&memory[i]) T (cr::forward <Args> (args)...);
       }
       return memory;
    }

   template <typename T> static void destructArray (T *memory, size_t length) noexcept {
       for (size_t i = 0; i < length; ++i) {
          memory[i].~T ();
       }
    }

   template <typename T, typename ...Args> static T *getAndConstruct (Args &&...args) noexcept {
      auto memory = get <T> ();
      construct <T> (memory, cr::forward <Args> (args)...);

      return memory;
   }

   template <typename T> static void transfer (T *dest, T *src, size_t length) noexcept {
      if constexpr (std::is_trivially_copyable_v <T>) {
         memcpy (dest, src, length * sizeof (T));
      }
      else {
         for (size_t i = 0; i < length; ++i) {
            construct <T> (&dest[i], cr::move (src[i]));
            destruct <T> (&src[i]);
         }
      }
   }
};

CR_NAMESPACE_END
