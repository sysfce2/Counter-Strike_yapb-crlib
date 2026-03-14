// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/basic.h>
#include <crlib/movable.h>

CR_NAMESPACE_BEGIN

// simple pair (twin)
template <typename A, typename B> class Twin final {
public:
   A first;
   B second;

public:
   constexpr Twin () : first (), second () {}
   ~Twin () = default;

   constexpr Twin (const A &a, const B &b) : first (a), second (b) {}

   template <typename T, typename U> constexpr Twin (T &&a, U &&b) : first (cr::forward <T> (a)), second (cr::forward <U> (b)) {}

   constexpr Twin (const Twin &) = default;
   constexpr Twin (Twin &&) noexcept = default;

   template <typename T, typename U> constexpr Twin (const Twin <T, U> &rhs) : first (rhs.first), second (rhs.second) {}
   template <typename T, typename U> constexpr Twin (Twin <T, U> &&rhs) noexcept : first (cr::forward <T> (rhs.first)), second (cr::forward <U> (rhs.second)) {}

public:
   constexpr Twin &operator = (const Twin &) = default;
   constexpr Twin &operator = (Twin &&) noexcept = default;

   template <typename T, typename U> constexpr Twin &operator = (const Twin <T, U> &rhs) {
      first = rhs.first;
      second = rhs.second;
      return *this;
   }

   template <typename T, typename U> constexpr Twin &operator = (Twin <T, U> &&rhs) noexcept {
      first = cr::forward <T> (rhs.first);
      second = cr::forward <U> (rhs.second);
      return *this;
   }

public:
   constexpr bool operator == (const Twin &rhs) const {
      return first == rhs.first && second == rhs.second;
   }

   constexpr bool operator != (const Twin &rhs) const {
      return !(*this == rhs);
   }

   constexpr bool operator < (const Twin &rhs) const {
      if (first < rhs.first) {
         return true;
      }
      if (rhs.first < first) {
         return false;
      }
      return second < rhs.second;
   }

   constexpr bool operator > (const Twin &rhs) const {
      return rhs < *this;
   }

   constexpr bool operator <= (const Twin &rhs) const {
      return !(rhs < *this);
   }

   constexpr bool operator >= (const Twin &rhs) const {
      return !(*this < rhs);
   }

public:
   constexpr void swap (Twin &rhs) noexcept {
      cr::swap (first, rhs.first);
      cr::swap (second, rhs.second);
   }
};

template <typename A, typename B> constexpr Twin <typename decay <A>::type, typename decay <B>::type> make_twin (A &&a, B &&b) {
   return Twin <typename decay <A>::type, typename decay <B>::type> (cr::forward <A> (a), cr::forward <B> (b));
}

template <typename A, typename B> constexpr void swap (Twin <A, B> &lhs, Twin <A, B> &rhs) noexcept {
   lhs.swap (rhs);
}

CR_NAMESPACE_END
