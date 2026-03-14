// benchmark_deque.cpp — benchmark Deque vs std::deque
#include <crlib/crlib.h>
#include <deque>
#include <vector>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

static constexpr size_t kNumElements = 1000;

TEST_CASE ("Deque benchmark", "[benchmark][deque]") {
   SECTION ("cr::Deque") {
      BENCHMARK ("push back") {
         Deque<int> dq;
         for (size_t i = 0; i < kNumElements; ++i) {
            dq.emplaceLast (static_cast <int> (i));
         }
         return dq;
      };

      BENCHMARK ("push front") {
         Deque<int> dq;
         for (size_t i = 0; i < kNumElements; ++i) {
            dq.emplaceFront (-static_cast <int> (i));
         }
         return dq;
      };

      BENCHMARK_ADVANCED ("front access + rotate")(Catch::Benchmark::Chronometer meter) {
         Deque<int> dq;
         for (size_t i = 0; i < kNumElements; ++i) {
            dq.emplaceLast (static_cast <int> (i));
         }
         meter.measure ([&] {
            int sum = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               sum += dq.front ();
               dq.discardFront ();
               dq.emplaceLast (static_cast <int> (i));
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("pop back")(Catch::Benchmark::Chronometer meter) {
         std::vector<Deque<int>> dqs;
         dqs.reserve (static_cast <size_t> (meter.runs ()));
         for (int r = 0; r < meter.runs (); ++r) {
            dqs.emplace_back ();
            for (size_t i = 0; i < kNumElements; ++i) {
               dqs.back ().emplaceLast (static_cast <int> (i));
            }
         }
         meter.measure ([&](int i) {
            for (size_t j = 0; j < kNumElements / 2; ++j) {
               dqs[i].popLast ();
            }
         });
      };

      BENCHMARK_ADVANCED ("pop front")(Catch::Benchmark::Chronometer meter) {
         std::vector<Deque<int>> dqs;
         dqs.reserve (static_cast <size_t> (meter.runs ()));
         for (int r = 0; r < meter.runs (); ++r) {
            dqs.emplace_back ();
            for (size_t i = 0; i < kNumElements; ++i) {
               dqs.back ().emplaceLast (static_cast <int> (i));
            }
         }
         meter.measure ([&](int i) {
            for (size_t j = 0; j < kNumElements / 2; ++j) {
               dqs[i].popFront ();
            }
         });
      };
   }

   SECTION ("std::deque") {
      BENCHMARK ("push_back") {
         std::deque<int> dq;
         for (size_t i = 0; i < kNumElements; ++i) {
            dq.push_back (static_cast <int> (i));
         }
         return dq;
      };

      BENCHMARK ("push_front") {
         std::deque<int> dq;
         for (size_t i = 0; i < kNumElements; ++i) {
            dq.push_front (-static_cast <int> (i));
         }
         return dq;
      };

      BENCHMARK_ADVANCED ("front access + rotate")(Catch::Benchmark::Chronometer meter) {
         std::deque<int> dq;
         for (size_t i = 0; i < kNumElements; ++i) {
            dq.push_back (static_cast <int> (i));
         }
         meter.measure ([&] {
            int sum = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               sum += dq.front ();
               dq.pop_front ();
               dq.push_back (static_cast <int> (i));
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("pop_back")(Catch::Benchmark::Chronometer meter) {
         std::vector<std::deque<int>> dqs (static_cast <size_t> (meter.runs ()));
         for (auto &d : dqs) {
            for (size_t i = 0; i < kNumElements; ++i) {
               d.push_back (static_cast <int> (i));
            }
         }
         meter.measure ([&](int i) {
            for (size_t j = 0; j < kNumElements / 2; ++j) {
               dqs[i].pop_back ();
            }
         });
      };

      BENCHMARK_ADVANCED ("pop_front")(Catch::Benchmark::Chronometer meter) {
         std::vector<std::deque<int>> dqs (static_cast <size_t> (meter.runs ()));
         for (auto &d : dqs) {
            for (size_t i = 0; i < kNumElements; ++i) {
               d.push_back (static_cast <int> (i));
            }
         }
         meter.measure ([&](int i) {
            for (size_t j = 0; j < kNumElements / 2; ++j) {
               dqs[i].pop_front ();
            }
         });
      };
   }
}
