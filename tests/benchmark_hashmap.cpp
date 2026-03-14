// benchmark_hashmap.cpp — benchmark HashMap vs std::unordered_map
#include <crlib/crlib.h>
#include <unordered_map>
#include <vector>
#include <numeric>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

static constexpr size_t kNumElements = 1000;

TEST_CASE ("HashMap benchmark", "[benchmark][hashmap]") {
   std::vector<int> keys (kNumElements);
   std::iota (keys.begin (), keys.end (), 0);

   SECTION ("cr::HashMap") {
      BENCHMARK_ADVANCED ("insert")(Catch::Benchmark::Chronometer meter) {
         meter.measure ([&] {
            HashMap<int, int> map;
            map.reserve (kNumElements * 2);
            for (size_t i = 0; i < kNumElements; ++i) {
               map.insert (keys[i], keys[i] * 2);
            }
            return map;
         });
      };

      BENCHMARK_ADVANCED ("lookup")(Catch::Benchmark::Chronometer meter) {
         HashMap<int, int> map;
         map.reserve (kNumElements * 2);
         for (size_t i = 0; i < kNumElements; ++i) {
            map.insert (keys[i], keys[i] * 2);
         }
         meter.measure ([&] {
            int sum = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               if (auto *val = map.find (keys[i])) {
                  sum += *val;
               }
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("erase half")(Catch::Benchmark::Chronometer meter) {
         std::vector<HashMap<int, int>> maps;
         maps.reserve (static_cast <size_t> (meter.runs ()));
         for (int r = 0; r < meter.runs (); ++r) {
            maps.emplace_back ();
            for (size_t i = 0; i < kNumElements; ++i) {
               maps.back ().insert (keys[i], keys[i] * 2);
            }
         }
         meter.measure ([&](int i) {
            for (size_t j = 0; j < kNumElements / 2; ++j) {
               maps[i].erase (keys[j]);
            }
         });
      };
   }

   SECTION ("std::unordered_map") {
      BENCHMARK_ADVANCED ("insert")(Catch::Benchmark::Chronometer meter) {
         meter.measure ([&] {
            std::unordered_map<int, int> map;
            for (size_t i = 0; i < kNumElements; ++i) {
               map.insert ({ keys[i], keys[i] * 2 });
            }
            return map;
         });
      };

      BENCHMARK_ADVANCED ("lookup")(Catch::Benchmark::Chronometer meter) {
         std::unordered_map<int, int> map;
         for (size_t i = 0; i < kNumElements; ++i) {
            map.insert ({ keys[i], keys[i] * 2 });
         }
         meter.measure ([&] {
            int sum = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               auto it = map.find (keys[i]);
               if (it != map.end ()) {
                  sum += it->second;
               }
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("erase half")(Catch::Benchmark::Chronometer meter) {
         std::vector<std::unordered_map<int, int>> maps (static_cast <size_t> (meter.runs ()));
         for (auto &m : maps) {
            for (size_t i = 0; i < kNumElements; ++i) {
               m.insert ({ keys[i], keys[i] * 2 });
            }
         }
         meter.measure ([&](int i) {
            for (size_t j = 0; j < kNumElements / 2; ++j) {
               maps[i].erase (keys[j]);
            }
         });
      };
   }
}
