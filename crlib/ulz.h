// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/array.h>

CR_NAMESPACE_BEGIN

// see https://github.com/encode84/ulz/
class ULZ final : public Singleton <ULZ> {
public:
   enum : int32_t {
      Excess = 16,
      UncompressFailure = -1
   };

private:
   enum : int32_t {
      WindowBits = 17,
      WindowSize = cr::bit (WindowBits),
      WindowMask = WindowSize - 1,

      MinMatch = 4,
      MaxChain = cr::bit (5),

      HashBits = 19,
      HashLength = cr::bit (HashBits),
      EmptyHash = -1,
   };

   SmallArray <int32_t> hashTable_ {};
   SmallArray <int32_t> prevTable_ {};

private:
   static uint32_t load16 (const uint8_t *ptr) {
      uint16_t ret;
      memcpy (&ret, ptr, sizeof (uint16_t));

      return ret;
   }

   static uint32_t load32 (const uint8_t *ptr) {
      uint32_t ret;
      memcpy (&ret, ptr, sizeof (uint32_t));

      return ret;
   }

   static void store16 (uint8_t *ptr, uint16_t val) {
      memcpy (ptr, &val, sizeof (uint16_t));
   }

   static void copy8 (uint8_t *dst, const uint8_t *src) {
      memcpy (dst, src, sizeof (uint64_t));
   }

   static void safeCopy (uint8_t *dst, const uint8_t *src, int32_t count) {
      memcpy (dst, src, count);
   }

   static void wildCopy (uint8_t *dst, const uint8_t *src, int32_t count) {
      if (count < 8) {
         for (int32_t i = 0; i < count; ++i) {
            dst[i] = src[i];
         }
         return;
      }
      copy8 (dst, src);

      for (int32_t i = 8; i < count; i += 8) {
         copy8 (dst + i, src + i);
      }
   }

   static uint32_t hash32 (const uint8_t *ptr) {
      return (load32 (ptr) * 0x9e3779b9) >> (32 - HashBits);
   }

   static void emitByte (uint8_t *&dst, int32_t val) {
      *dst++ = static_cast <uint8_t> (val);
   }

   static void encodeVarInt (uint8_t *&ptr, uint32_t val) {
      while (val >= 128) {
         val -= 128;

         *ptr++ = 128 + (val & 127);
         val >>= 7;
      }
      *ptr++ = static_cast <uint8_t> (val);
   }

   static uint32_t decodeVarInt (const uint8_t *&ptr, const uint8_t *end) {
      uint32_t val = 0;

      for (int32_t i = 0; i <= 21; i += 7) {
         if (ptr >= end) {
            return val;
         }
         const uint32_t cur = *ptr++;
         val += cur << i;

         if (cur < 128) {
            break;
         }
      }
      return val;
   }

   void updateChain (const uint8_t *in, int32_t pos) {
      const auto hash = hash32 (&in[pos]);

      prevTable_[pos & WindowMask] = hashTable_[hash];
      hashTable_[hash] = pos;
   }

   int32_t findBestMatch (const uint8_t *in, int32_t cur, int32_t maxMatch, int32_t &dist) const {
      const auto limit = cr::max <int32_t> (cur - WindowSize, EmptyHash);

      int32_t chainLength = MaxChain;
      int32_t lookup = hashTable_[hash32 (&in[cur])];
      int32_t bestLength = 0;

      while (lookup > limit) {
         if (in[lookup + bestLength] == in[cur + bestLength] && load32 (&in[lookup]) == load32 (&in[cur])) {
            int32_t length = MinMatch;

            while (length < maxMatch && in[lookup + length] == in[cur + length]) {
               ++length;
            }

            if (length > bestLength) {
               bestLength = length;
               dist = cur - lookup;

               if (length == maxMatch) {
                  break;
               }
            }
         }

         if (--chainLength == 0) {
            break;
         }
         lookup = prevTable_[lookup & WindowMask];
      }
      return bestLength;
   }

   bool hasLazyMatch (const uint8_t *in, int32_t next, int32_t target) const {
      const auto limit = cr::max <int32_t> (next - WindowSize, EmptyHash);

      int32_t chainLength = MaxChain;
      int32_t lookup = hashTable_[hash32 (&in[next])];

      while (lookup > limit) {
         if (in[lookup + target - 1] == in[next + target - 1] && load32 (&in[lookup]) == load32 (&in[next])) {
            int32_t length = MinMatch;

            while (length < target && in[lookup + length] == in[next + length]) {
               ++length;
            }

            if (length == target) {
               return true;
            }
         }

         if (--chainLength == 0) {
            break;
         }
         lookup = prevTable_[lookup & WindowMask];
      }
      return false;
   }

   void emitLiterals (uint8_t *&op, const uint8_t *in, int32_t anchor, int32_t cur, int32_t token) {
      const auto run = cur - anchor;

      if (run >= 7) {
         emitByte (op, (7 << 5) + token);
         encodeVarInt (op, run - 7);
      }
      else {
         emitByte (op, (run << 5) + token);
      }
      safeCopy (op, &in[anchor], run);
      op += run;
   }

public:
   explicit ULZ () {
      hashTable_.resize (HashLength);
      prevTable_.resize (WindowSize);
   }

   ~ULZ () = default;

public:
   int32_t compress (const uint8_t *in, int32_t inputLength, uint8_t *out) {
      for (auto &htb : hashTable_) {
         htb = EmptyHash;
      }
      auto op = out;

      int32_t anchor = 0;
      int32_t cur = 0;

      while (cur < inputLength) {
         const int32_t maxMatch = inputLength - cur;

         int32_t dist = 0;
         int32_t bestLength = 0;

         if (maxMatch >= MinMatch) {
            bestLength = findBestMatch (in, cur, maxMatch, dist);
         }

         // skip short matches when literal run is long
         if (bestLength == MinMatch && (cur - anchor) >= (7 + 128)) {
            bestLength = 0;
         }

         // lazy matching: check if next position yields a better match
         if (bestLength >= MinMatch && bestLength < maxMatch && (cur - anchor) != 6) {
            if (hasLazyMatch (in, cur + 1, bestLength + 1)) {
               bestLength = 0;
            }
         }

         if (bestLength >= MinMatch) {
            const auto length = bestLength - MinMatch;
            const auto token = ((dist >> 12) & 16) + cr::min <int32_t> (length, 15);

            if (anchor != cur) {
               emitLiterals (op, in, anchor, cur, token);
            }
            else {
               emitByte (op, token);
            }

            if (length >= 15) {
               encodeVarInt (op, length - 15);
            }
            store16 (op, static_cast <uint16_t> (dist));
            op += 2;

            while (bestLength-- != 0) {
               updateChain (in, cur);
               ++cur;
            }
            anchor = cur;
         }
         else {
            updateChain (in, cur);
            ++cur;
         }
      }

      if (anchor != cur) {
         const auto run = cur - anchor;

         if (run >= 7) {
            emitByte (op, 7 << 5);
            encodeVarInt (op, run - 7);
         }
         else {
            emitByte (op, run << 5);
         }
         safeCopy (op, &in[anchor], run);
         op += run;
      }
      return static_cast <int32_t> (op - out);
   }

   int32_t uncompress (const uint8_t *in, int32_t inputLength, uint8_t *out, int32_t outLength) {
      auto op = out;
      auto ip = in;

      const auto opEnd = op + outLength;
      const auto ipEnd = ip + inputLength;

      while (ip < ipEnd) {
         const auto token = *ip++;

         if (token >= 32) {
            auto run = static_cast <int32_t> (token >> 5);

            if (run == 7) {
               run += decodeVarInt (ip, ipEnd);
            }

            if ((opEnd - op) < run || (ipEnd - ip) < run) {
               return UncompressFailure;
            }
            safeCopy (op, ip, run);

            op += run;
            ip += run;

            if (ip >= ipEnd) {
               break;
            }
         }

         auto length = static_cast <int32_t> ((token & 15) + MinMatch);

         if (length == (15 + MinMatch)) {
            length += decodeVarInt (ip, ipEnd);
         }

         if ((ipEnd - ip) < 2) {
            return UncompressFailure;
         }

         if ((opEnd - op) < length) {
            return UncompressFailure;
         }
         const auto dist = static_cast <int32_t> (((token & 16) << 12) + load16 (ip));
         ip += 2;

         if (dist == 0 || (op - out) < dist) {
            return UncompressFailure;
         }
         auto cp = op - dist;

         if (dist >= 8) {
            wildCopy (op, cp, length);
            op += length;
         }
         else {
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;

            while (length-- != 4) {
               *op++ = *cp++;
            }
         }
      }
      return (ip == ipEnd) ? static_cast <int32_t> (op - out) : UncompressFailure;
   }
};

CR_NAMESPACE_END
