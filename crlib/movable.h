// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/traits.h>

CR_NAMESPACE_BEGIN

template <typename T> remove_reference_t <T> constexpr &&move (T &&type) noexcept {
   return static_cast <remove_reference_t <T> &&> (type);
}

template <typename T> constexpr T &&forward (remove_reference_t <T> &type) noexcept {
   return static_cast <T &&> (type);
}

template <typename T> constexpr T &&forward (remove_reference_t <T> &&type) noexcept {
   return static_cast <T &&> (type);
}

template <typename T> constexpr void swap (T &left, T &right) noexcept {
   auto temp = cr::move (left);
   left = cr::move (right);
   right = cr::move (temp);
}

template <typename T, size_t S> constexpr void swap (T (&left)[S], T (&right)[S]) noexcept {
   if (&left == &right) {
      return;
   }
   auto begin = left;
   auto end = begin + S;

   for (auto temp = right; begin != end; ++begin, ++temp) {
      cr::swap (*begin, *temp);
   }
}

CR_NAMESPACE_END
