//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>
#include <crlib/traits.h>

CR_NAMESPACE_BEGIN

// random number generator, credits goes to https://prng.di.unimi.it/xoshiro128starstar.c
class Xoshiro128 final : public Singleton <Xoshiro128> {
private:
   uint32_t states_[4] {};
   uint32_t states2_[4] {};
   uint64_t limit_ { static_cast <uint64_t> (1) << 32ull };

private:
   constexpr uint32_t rotl32 (const uint32_t x, int32_t k) {
      return (x << k) | (x >> (32 - k));
   }

   constexpr uint32_t splitmix32 (uint32_t &x) {
      uint32_t z = (x += 0x9e3779b9);
      z = (z ^ (z >> 16)) * 0x85ebca6b;
      z = (z ^ (z >> 13)) * 0xc2b2ae35;
      return z ^ (z >> 16);
   }

   constexpr void seeder (uint32_t *state) {
      uint32_t seed = 0;

      state[0] = splitmix32 (seed);
      state[1] = splitmix32 (seed);
      state[2] = splitmix32 (seed);
      state[3] = splitmix32 (seed);
   }

private:
   constexpr uint32_t scramble (uint32_t *states, const uint32_t result) {
      const uint32_t t = states[1] << 9;

      states[2] ^= states[0];
      states[3] ^= states[1];
      states[1] ^= states[2];
      states[0] ^= states[3];
      states[2] ^= t;
      states[3] = rotl32 (states[3], 11);

      return result;
   }

   template <typename U> constexpr uint32_t next () {
      if constexpr (cr::is_same <U, int32_t>::value) {
         return scramble (states_, rotl32 (states_[0] + states_[3], 7) + states_[0]);
      }
      else if constexpr (cr::is_same <U, float>::value) {
         return scramble (states2_, states2_[0] + states2_[3]);
      }
   }

public:
   constexpr explicit Xoshiro128 () {
      seeder (states_);
      seeder (states2_);
   }

public:
   template <typename U, typename Void = void> constexpr U get (U, U) = delete;

   template <typename Void = void> constexpr int32_t get (int32_t low, int32_t high) {
      return static_cast <int32_t> (next <int32_t> () * (static_cast <double> (high) - static_cast <double> (low) + 1.0) / limit_ + static_cast <double> (low));
   }

   template <typename Void = void> constexpr float get (float low, float high) {
      return static_cast <float> (next <float> () * (static_cast <double> (high) - static_cast <double> (low)) / (limit_ - 1) + static_cast <double> (low));
   }

public:
   constexpr bool chance (int32_t limit) {
      return get <int32_t> (0, 100) < limit;
   }
};

// expose global random generator
CR_EXPOSE_GLOBAL_SINGLETON (Xoshiro128, rg);

CR_NAMESPACE_END
