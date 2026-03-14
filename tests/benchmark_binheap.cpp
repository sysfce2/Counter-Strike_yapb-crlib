// benchmark_binheap.cpp — benchmark BinaryHeap vs std::priority_queue
#include <crlib/crlib.h>
#include <queue>
#include <vector>
#include <numeric>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

static constexpr size_t kNumElements = 1000;

TEST_CASE ("BinaryHeap benchmark", "[benchmark][binheap]") {
   std::vector<int> values (kNumElements);
   std::iota (values.begin (), values.end (), 0);

   SECTION ("cr::BinaryHeap") {
      BENCHMARK_ADVANCED ("push")(Catch::Benchmark::Chronometer meter) {
         meter.measure ([&] {
            BinaryHeap<int> heap;
            for (size_t i = 0; i < kNumElements; ++i) {
               heap.push (values[i]);
            }
            return heap;
         });
      };

      BENCHMARK_ADVANCED ("top")(Catch::Benchmark::Chronometer meter) {
         BinaryHeap<int> heap;
         for (size_t i = 0; i < kNumElements; ++i) {
            heap.push (values[i]);
         }
         meter.measure ([&] {
            return heap.top ();
         });
      };

      BENCHMARK_ADVANCED ("pop half")(Catch::Benchmark::Chronometer meter) {
         std::vector<BinaryHeap<int>> heaps;
         heaps.reserve (static_cast <size_t> (meter.runs ()));
         for (int r = 0; r < meter.runs (); ++r) {
            heaps.emplace_back ();
            for (size_t i = 0; i < kNumElements; ++i) {
               heaps.back ().push (values[i]);
            }
         }
         meter.measure ([&](int i) {
            for (size_t j = 0; j < kNumElements / 2; ++j) {
               heaps[i].pop ();
            }
         });
      };
   }

   SECTION ("std::priority_queue") {
      BENCHMARK_ADVANCED ("push")(Catch::Benchmark::Chronometer meter) {
         meter.measure ([&] {
            std::priority_queue<int> heap;
            for (size_t i = 0; i < kNumElements; ++i) {
               heap.push (values[i]);
            }
            return heap;
         });
      };

      BENCHMARK_ADVANCED ("top")(Catch::Benchmark::Chronometer meter) {
         std::priority_queue<int> heap;
         for (size_t i = 0; i < kNumElements; ++i) {
            heap.push (values[i]);
         }
         meter.measure ([&] {
            return heap.top ();
         });
      };

      BENCHMARK_ADVANCED ("pop half")(Catch::Benchmark::Chronometer meter) {
         std::vector<std::priority_queue<int>> heaps (static_cast <size_t> (meter.runs ()));
         for (auto &h : heaps) {
            for (size_t i = 0; i < kNumElements; ++i) {
               h.push (values[i]);
            }
         }
         meter.measure ([&](int i) {
            for (size_t j = 0; j < kNumElements / 2; ++j) {
               heaps[i].pop ();
            }
         });
      };
   }
}
