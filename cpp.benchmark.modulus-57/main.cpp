#include <benchmark/benchmark.h>

using namespace std;

static void shift(benchmark::State &state) {
  char32_t val;
  for (auto _ : state) {
    val >>= 10;
    benchmark::DoNotOptimize(val);
  }
}
BENCHMARK(shift);

static void v57(benchmark::State &state) {
  char32_t val;
  for (auto _ : state) {
    val %= 57;
    benchmark::DoNotOptimize(val);
  }
}
BENCHMARK(v57);

static void v41(benchmark::State &state) {
  char32_t val;
  for (auto _ : state) {
    val %= 41;
    benchmark::DoNotOptimize(val);
  }
}
BENCHMARK(v41);

static void v111(benchmark::State &state) {
  char32_t val;
  for (auto _ : state) {
    val %= 111;
    benchmark::DoNotOptimize(val);
  }
}
BENCHMARK(v111);

static void v57045(benchmark::State &state) {
  char32_t val;
  for (auto _ : state) {
    val %= 57045;
    benchmark::DoNotOptimize(val);
  }
}
BENCHMARK(v57045);

static void v2(benchmark::State &state) {
  char32_t val;
  for (auto _ : state) {
    val %= 2;
    benchmark::DoNotOptimize(val);
  }
}
BENCHMARK(v2);

static void v60(benchmark::State &state) {
  char32_t val;
  for (auto _ : state) {
    val %= 60;
    benchmark::DoNotOptimize(val);
  }
}
BENCHMARK(v60);
