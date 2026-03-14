// benchmark_string.cpp — benchmark String vs std::string
#include <crlib/crlib.h>
#include <string>
#include <vector>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

static constexpr size_t kNumElements = 1000;
static constexpr const char *kTestString = "test_item_value_with_a_padding_to_my_item_value_for_test_test_value_prevent_sso_of_std_string_or_whatever_sso_stuff";
static constexpr const char *kFindPattern = "my_item_value_for_test_test_value";

TEST_CASE ("String benchmark", "[benchmark][string]") {
   SECTION ("cr::String") {
      BENCHMARK ("construct") {
         Array<String> strings;
         for (size_t i = 0; i < kNumElements; ++i) {
            strings.push (String (kTestString));
         }
         return strings;
      };

      BENCHMARK_ADVANCED ("concat")(Catch::Benchmark::Chronometer meter) {
         Array<String> strings;
         for (size_t i = 0; i < kNumElements; ++i) {
            strings.push (String (kTestString));
         }
         meter.measure ([&] {
            String combined;
            for (size_t i = 0; i < kNumElements / 10; ++i) {
               combined += strings[i];
            }
            return combined;
         });
      };

      BENCHMARK_ADVANCED ("char access")(Catch::Benchmark::Chronometer meter) {
         Array<String> strings;
         for (size_t i = 0; i < kNumElements; ++i) {
            strings.push (String (kTestString));
         }
         meter.measure ([&] {
            int sum = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               sum += strings[i][0];
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("find")(Catch::Benchmark::Chronometer meter) {
         Array<String> strings;
         for (size_t i = 0; i < kNumElements; ++i) {
            strings.push (String (kTestString));
         }
         meter.measure ([&] {
            int count = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               if (strings[i].find (kFindPattern) != String::InvalidIndex) {
                  ++count;
               }
            }
            return count;
         });
      };
   }

   SECTION ("std::string") {
      BENCHMARK ("construct") {
         std::vector<std::string> strings;
         for (size_t i = 0; i < kNumElements; ++i) {
            strings.push_back (std::string (kTestString));
         }
         return strings;
      };

      BENCHMARK_ADVANCED ("concat")(Catch::Benchmark::Chronometer meter) {
         std::vector<std::string> strings;
         for (size_t i = 0; i < kNumElements; ++i) {
            strings.push_back (std::string (kTestString));
         }
         meter.measure ([&] {
            std::string combined;
            for (size_t i = 0; i < kNumElements / 10; ++i) {
               combined += strings[i];
            }
            return combined;
         });
      };

      BENCHMARK_ADVANCED ("char access")(Catch::Benchmark::Chronometer meter) {
         std::vector<std::string> strings;
         for (size_t i = 0; i < kNumElements; ++i) {
            strings.push_back (std::string (kTestString));
         }
         meter.measure ([&] {
            int sum = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               sum += strings[i][0];
            }
            return sum;
         });
      };

      BENCHMARK_ADVANCED ("find")(Catch::Benchmark::Chronometer meter) {
         std::vector<std::string> strings;
         for (size_t i = 0; i < kNumElements; ++i) {
            strings.push_back (std::string (kTestString));
         }
         meter.measure ([&] {
            int count = 0;
            for (size_t i = 0; i < kNumElements; ++i) {
               if (strings[i].find (kFindPattern) != std::string::npos) {
                  ++count;
               }
            }
            return count;
         });
      };
   }
}
