// benchmark_array.cpp — benchmark Array vs std::vector
#include <crlib/crlib.h>
#include <vector>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

static constexpr size_t kNumElements = 1000;

TEST_CASE ("Array benchmark", "[benchmark][array]") {
   SECTION ("cr::Array") {
      BENCHMARK ("push") {
         Array<int> arr;
         for (size_t i = 0; i < kNumElements; ++i) {
            arr.push (static_cast <int> (i));
         }
         return arr;
      };

      BENCHMARK_ADVANCED ("random access")(Catch::Benchmark::Chronometer meter) {
         Array<int> arr;
         for (size_t i = 0; i < kNumElements; ++i) {
            arr.push (static_cast <int> (i));
         }
         meter.measure ([&] {
            int sum = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               sum += arr[i];
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("iteration")(Catch::Benchmark::Chronometer meter) {
         Array<int> arr;
         for (size_t i = 0; i < kNumElements; ++i) {
            arr.push (static_cast <int> (i));
         }
         meter.measure ([&] {
            int sum = 0;
            for (auto &v : arr) {
               sum += v;
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("erase half")(Catch::Benchmark::Chronometer meter) {
         std::vector<Array<int>> arrs;
         arrs.reserve (static_cast <size_t> (meter.runs ()));
         for (int r = 0; r < meter.runs (); ++r) {
            arrs.emplace_back ();
            for (size_t i = 0; i < kNumElements; ++i) {
               arrs.back ().push (static_cast <int> (i));
            }
         }
         meter.measure ([&](int i) {
            arrs[i].erase (0, kNumElements / 2);
         });
      };
   }

   SECTION ("std::vector") {
      BENCHMARK ("push_back") {
         std::vector<int> vec;
         for (size_t i = 0; i < kNumElements; ++i) {
            vec.push_back (static_cast <int> (i));
         }
         return vec;
      };

      BENCHMARK_ADVANCED ("random access")(Catch::Benchmark::Chronometer meter) {
         std::vector<int> vec;
         for (size_t i = 0; i < kNumElements; ++i) {
            vec.push_back (static_cast <int> (i));
         }
         meter.measure ([&] {
            int sum = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               sum += vec[i];
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("iteration")(Catch::Benchmark::Chronometer meter) {
         std::vector<int> vec;
         for (size_t i = 0; i < kNumElements; ++i) {
            vec.push_back (static_cast <int> (i));
         }
         meter.measure ([&] {
            int sum = 0;
            for (auto &v : vec) {
               sum += v;
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("erase half")(Catch::Benchmark::Chronometer meter) {
         std::vector<std::vector<int>> vecs (static_cast <size_t> (meter.runs ()));
         for (auto &v : vecs) {
            for (size_t i = 0; i < kNumElements; ++i) {
               v.push_back (static_cast <int> (i));
            }
         }
         meter.measure ([&](int i) {
            vecs[i].erase (vecs[i].begin (), vecs[i].begin () + kNumElements / 2);
         });
      };
   }
}
