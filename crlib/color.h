//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>

CR_NAMESPACE_BEGIN

// simple color holder
class Color final {
public:
   int32_t red = 0, green = 0, blue = 0;

public:
   Color (int32_t r, int32_t g, int32_t b) : red (r), green (g), blue (b) { }

   explicit Color () = default;
   ~Color () = default;

public:
   void reset () {
      red = green = blue = 0;
   }

   int32_t avg () const {
      return sum () / (sizeof (Color) / sizeof (int32_t));
   }

   int32_t sum () const {
      return red + green + blue;
   }
};

CR_NAMESPACE_END
