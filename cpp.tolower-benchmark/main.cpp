#include <benchmark/benchmark.h>

#if __has_include("/webpp/webpp/strings/to_case.hpp")
#define HAS_WEBPP
#include "/webpp/webpp/strings/to_case.hpp"
#endif

#include <cstdint>
#include <cstring>

#include <algorithm>
#include <cctype>
#include <string>

using namespace std;

// from:
// https://github.com/ada-url/idna/blob/fff988508f659ef5c6494572ebea3d5db2466ed0/src/mapping.cpp#L59
void ascii_map(char *input, size_t length) {
  auto broadcast = [](uint8_t v) -> uint64_t {
    return 0x101010101010101ull * v;
  };
  uint64_t broadcast_80 = broadcast(0x80);
  uint64_t broadcast_Ap = broadcast(128 - 'A');
  uint64_t broadcast_Zp = broadcast(128 - 'Z' - 1);
  size_t i = 0;

  for (; i + 7 < length; i += 8) {
    uint64_t word{};
    memcpy(&word, input + i, sizeof(word));
    word ^=
        (((word + broadcast_Ap) ^ (word + broadcast_Zp)) & broadcast_80) >> 2;
    memcpy(input + i, &word, sizeof(word));
  }
  if (i < length) {
    uint64_t word{};
    memcpy(&word, input + i, length - i);
    word ^=
        (((word + broadcast_Ap) ^ (word + broadcast_Zp)) & broadcast_80) >> 2;
    memcpy(input + i, &word, length - i);
  }
}

// AI generated
void to_lower(std::string &str) {
  std::transform(
      str.begin(), str.end(), str.begin(),
      [](unsigned char c) -> unsigned char { return std::tolower(c); });
}

static void AI(benchmark::State &state) {
  std::string str = "THIS Is a string, a very very very very very long long "
                    "long long string.";
  for (auto _ : state) {
    to_lower(str);
    benchmark::DoNotOptimize(str);
  }
}
BENCHMARK(AI);

static void IDNA(benchmark::State &state) {
  std::string str = "THIS Is a string, a very very very very very long long "
                    "long long string.";
  for (auto _ : state) {
    ascii_map(str.data(), str.size());
    benchmark::DoNotOptimize(str);
  }
}
BENCHMARK(IDNA);

#ifdef HAS_WEBPP
static void Webpp(benchmark::State &state) {
  std::string str = "THIS Is a string, a very very very very very long long "
                    "long long string.";
  for (auto _ : state) {
    webpp::ascii::lower_to(str, str.begin(), str.end());
    benchmark::DoNotOptimize(str);
  }
}
BENCHMARK(Webpp);
#endif
