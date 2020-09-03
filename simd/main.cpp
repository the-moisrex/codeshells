#include <benchmark/benchmark.h>
using namespace std;


static void EmptyRandom(benchmark::State& state) {
  for (auto _ : state) {
    int arr[4] = {};
    for (int i = 0; i < 1000; i++) {
      for (int j = 0; j < 4; ++j)
        arr[j] = i;
    }
    benchmark::DoNotOptimize(arr);
  }
}
BENCHMARK(EmptyRandom);


static void SIMD(benchmark::State& state) {
  for (auto _ : state) {
    int arr[4] = {};
    for (int i = 0; i < 1000; i+=4) {
      arr[0] = i;
      arr[1] = i;
      arr[2] = i;
      arr [3] = i;
    }
    benchmark::DoNotOptimize(arr);
  }
}
BENCHMARK(SIMD);

