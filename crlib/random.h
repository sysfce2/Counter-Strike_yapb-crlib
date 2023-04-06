//
// YaPB - Counter-Strike Bot based on PODBot by Markus Klinge.
// Copyright © 2004-2022 YaPB Project <yapb@jeefo.net>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>

CR_NAMESPACE_BEGIN

// based on: https://github.com/jeudesprits/PSWyhash/blob/master/Sources/CWyhash/include/wyhash.h
class Random : public Singleton <Random> {
private:
   uint64_t div_ { static_cast <uint64_t> (1) << 32ull };
   uint64_t state_ { static_cast <uint64_t> (time (nullptr)) };

public:
   explicit Random () {
      warmup ();
   }
   ~Random () = default;

private:
   uint64_t wyrand64 () {
      constexpr uint64_t wyp0 = 0xa0761d6478bd642full, wyp1 = 0xe7037ed1a0b428dbull;
      state_ += wyp0;

      return mul (state_ ^ wyp1, state_);
   }

   uint32_t wyrand32 () {
      return static_cast <uint32_t> (wyrand64 ());
   }

   void warmup () {
      for (auto i = 0; i < 32; ++i) {
         state_ ^= wyrand64 ();
      }
   }

private:
   uint64_t rotr (uint64_t v, uint32_t k) {
      return (v >> k) | (v << (64 - k));
   }

   uint64_t mul (uint64_t a, uint64_t b) {
      uint64_t hh = (a >> 32) * (b >> 32);
      uint64_t hl = (b >> 32) * static_cast <uint32_t> (b);

      uint64_t lh = static_cast <uint32_t> (a) * (b >> 32);
      uint64_t ll = static_cast <uint64_t> (static_cast <double> (a) * static_cast <double> (b));

      return rotr (hl, 32) ^ rotr (lh, 32) ^ hh ^ ll;
   }

public:
   template <typename U, typename Void = void> U get (U, U) = delete;

   template <typename Void = void> int32_t get (int32_t low, int32_t high) {
      return static_cast <int32_t> (wyrand32 () * (static_cast <double> (high) - static_cast <double> (low) + 1.0) / div_ + static_cast <double> (low));
   }

   template <typename Void = void> float get (float low, float high) {
      return static_cast <float> (wyrand32 () * (static_cast <double> (high) - static_cast <double> (low)) / (div_ - 1) + static_cast <double> (low));
   }

public:
   bool chance (int32_t limit) {
      return get <int32_t> (0, 100) < limit;
   }
};

// expose global random generator
CR_EXPOSE_GLOBAL_SINGLETON (Random, rg);

CR_NAMESPACE_END
