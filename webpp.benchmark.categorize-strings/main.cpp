#include <benchmark/benchmark.h>
#include <chrono>
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include "./charset.hpp"

using namespace std;
using namespace webpp;

// Characters to check for
const unordered_set<char> chars_to_check = {'.', ',', ';', ':', '!'};

        using flag_type = stl::uint_fast8_t;
        enum struct flag_types : flag_type {
            // ASCII and Non-ASCII:
            non_ascii   = 0b1000U,
            ascii       = 0b1'0000U,
            ascii_upper = 0b10'0000U | ascii,
            dot         = 0b100'0000U | ascii,

            // xn-- (Called ACE Prefix):
            x    = 0b1U | ascii,
            n    = 0b10U | ascii,
            dash = 0b100U | ascii,
            ace  = x | n | dash, // ACE prefix

            // Misc:
            clean         = static_cast<flag_type>(~dot | ascii),
            length_police = (dot | non_ascii) & ~ascii,
            all           = 0b1111'1111U, // all possibilities
        };

        // array<flag_types, 256>
        static constexpr auto interesting_characters = categorize<256U>(
          cat{.set = ".,;:!", .value = flag_types::dot}
          );


// Method 1: Using switch statement
bool contains_special_switch(const string &s) {
  for (char c : s) {
    switch (c) {
    case '.':
    case ',':
    case ';':
    case ':':
    case '!':
      return true;
    default:
      break;
    }
  }
  return false;
}

// Method 2: Using table lookup (unordered_set)
bool contains_special_lookup(const string &s) {
  for (char c : s) {
    if (chars_to_check.find(c) != chars_to_check.end()) {
      return true;
    }
  }
  return false;
}

bool contains_table_lookup(const string &s) {
  using std::to_underlying;
  using enum flag_types;
  auto spos = s.begin();
              flag_type flag = or_all_if<flag_type>(
              interesting_characters,
              spos,
              s.end(),
              [](flag_type const cur_flags) constexpr noexcept -> bool {
                  return cur_flags >= to_underlying(dot); // we found a dot
              });
  return flag == to_underlying(dot); // don't really matter
}

bool contains_table_lookup_all(const string &s) {
  using std::to_underlying;
  using enum flag_types;
  auto spos = s.begin();
              flag_type flag = or_all<flag_type>(
              interesting_characters,
              spos,
              s.end());
  return flag == to_underlying(dot); // don't really matter
}

static void BM_StringSwitch(benchmark::State &state) {
  std::string test_string(1000, 'a');
  test_string[123] = '.';
  test_string[654] = '!';
  for (auto _ : state) {
    benchmark::DoNotOptimize(contains_special_switch(test_string));
  }
}
BENCHMARK(BM_StringSwitch);

static void BM_StringLookup(benchmark::State &state) {
  std::string test_string(1000, 'a');
  test_string[123] = '.';
  test_string[654] = '!';
  for (auto _ : state) {
    benchmark::DoNotOptimize(contains_special_lookup(test_string));
  }
}
BENCHMARK(BM_StringLookup);

static void BM_TableLookup(benchmark::State &state) {
  std::string test_string(1000, 'a');
  test_string[123] = '.';
  test_string[654] = '!';
  for (auto _ : state) {
    benchmark::DoNotOptimize(contains_table_lookup(test_string));
  }
}
BENCHMARK(BM_TableLookup);

static void BM_TableLookupAll(benchmark::State &state) {
  std::string test_string(1000, 'a');
  test_string[123] = '.';
  test_string[654] = '!';
  for (auto _ : state) {
    benchmark::DoNotOptimize(contains_table_lookup_all(test_string));
  }
}
BENCHMARK(BM_TableLookupAll);