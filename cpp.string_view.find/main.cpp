#include <benchmark/benchmark.h>

using namespace std;

static void EmptyRandom(benchmark::State &state) {
  for (auto _ : state) {
    // benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(EmptyRandom);

string str =
    "long string 1 long string 1long string 1long string 1long string 1long "
    "string 1long string 1long string 1long string 1long string 1long string "
    "1long string 1long string 1long string 1long string 1long string 1long "
    "string 1long string 1long string 1long string 1long string 1long string "
    "1long string 1long string 1long string 1long string 1long string 1long "
    "string 1long string 1long string 1long string 1long string 1long string "
    "1long string 1long string 1long string 1long string 1long string 1long "
    "string 1long string 1long string 1long string 1long string 1long string "
    "1long string 1long string 1long string 1long string 1long string 1long "
    "string 1long string 1long string 1long string 1long string 1long string "
    "1long string 1long string 1long string 1long string 1long string 1long "
    "string 1 *";
static void FindStr(benchmark::State &state) {
  str += " oh oh";
  for (auto _ : state) {
    const auto c = str.find("*");
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(FindStr);

static void FindChar(benchmark::State &state) {
  for (auto _ : state) {
    const auto c = str.find('*');
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(FindChar);

static void FindStrView(benchmark::State &state) {
  const string_view star{"*"};
  for (auto _ : state) {
    const auto c = str.find(star);
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(FindStrView);

// BENCHMARK_MAIN();
