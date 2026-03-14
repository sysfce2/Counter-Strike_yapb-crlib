// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/basic.h>

CR_NAMESPACE_BEGIN

// simple color holder
class Color final {
public:
   int32_t red {}, green {}, blue {};

public:
   constexpr Color (int32_t r, int32_t g, int32_t b) : red (r), green (g), blue (b) { }

   explicit constexpr Color () = default;
   ~Color () = default;

public:
   constexpr void reset () {
      red = green = blue = 0;
   }

   constexpr int32_t avg () const {
      return sum () / 3;
   }

   constexpr int32_t sum () const {
      return red + green + blue;
   }

   constexpr bool operator == (const Color &rhs) const {
      return red == rhs.red && green == rhs.green && blue == rhs.blue;
   }

   constexpr bool operator != (const Color &rhs) const {
      return !operator == (rhs);
   }

   constexpr Color operator + (const Color &rhs) const {
      return Color (red + rhs.red, green + rhs.green, blue + rhs.blue);
   }

   constexpr Color operator - (const Color &rhs) const {
      return Color (red - rhs.red, green - rhs.green, blue - rhs.blue);
   }

   constexpr Color operator * (int32_t scalar) const {
      return Color (red * scalar, green * scalar, blue * scalar);
   }

   constexpr Color operator / (int32_t scalar) const {
      return Color (red / scalar, green / scalar, blue / scalar);
   }

   constexpr Color &operator += (const Color &rhs) {
      red += rhs.red;
      green += rhs.green;
      blue += rhs.blue;
      return *this;
   }

   constexpr Color &operator -= (const Color &rhs) {
      red -= rhs.red;
      green -= rhs.green;
      blue -= rhs.blue;
      return *this;
   }

   constexpr Color &operator *= (int32_t scalar) {
      red *= scalar;
      green *= scalar;
      blue *= scalar;
      return *this;
   }

   constexpr Color &operator /= (int32_t scalar) {
      red /= scalar;
      green /= scalar;
      blue /= scalar;
      return *this;
   }

   constexpr Color clamped () const {
      auto clamp = [] (int32_t v) constexpr {
         return v < 0 ? 0 : (v > 255 ? 255 : v);
      };
      return Color (clamp (red), clamp (green), clamp (blue));
   }

   constexpr void clamp () {
      auto c = clamped ();
      red = c.red;
      green = c.green;
      blue = c.blue;
   }
};

CR_NAMESPACE_END
