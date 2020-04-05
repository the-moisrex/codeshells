#include <benchmark/benchmark.h>

using namespace std;

static void Raw(benchmark::State &state) {

  for (auto _ : state) {
    int c = 1;
    for (int i = 0; i < 100000; i++) {
      c *= i;
      if (c < i)
        benchmark::DoNotOptimize(c);
    }
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(Raw);

static void RawTryCatch(benchmark::State &state) {

  for (auto _ : state) {
    int c = 1;
    try {
      for (int i = 0; i < 100000; i++) {
        c *= i;
        if (c < i)
          benchmark::DoNotOptimize(c);
      }
      benchmark::DoNotOptimize(c);
    } catch (std::exception const &err) {
      benchmark::DoNotOptimize(err);
    } catch (...) {
      benchmark::DoNotOptimize(c);
    }
  }
}
BENCHMARK(RawTryCatch);

static void NoRawTryCatch(benchmark::State &state) {

  for (auto _ : state) {
    int c = 1;
    for (int i = 0; i < 100000; i++) {
      try {
        c *= i;
        if (c < i)
          throw 0;
      } catch (...) {
        benchmark::DoNotOptimize(c);
      }
    }
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(NoRawTryCatch);

// BENCHMARK_MAIN();
