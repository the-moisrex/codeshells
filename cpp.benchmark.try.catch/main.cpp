#include <benchmark/benchmark.h>

using namespace std;

static void EmptyRandom(benchmark::State& state) {

  for (auto _ : state) {

    // benchmark::DoNotOptimize(c);
  }

}
// Register the function as a benchmark
BENCHMARK(EmptyRandom);


static void Try(benchmark::State& state) {
    int a = 0;
  for (auto _ : state) {
    try {
      benchmark::DoNotOptimize(a++);
      if (a % 2 == 0) {
          throw std::invalid_argument("nice");
      }
    } catch(...) {
      benchmark::DoNotOptimize(a++);
    }
  }
}
BENCHMARK(Try);


static void NoTry(benchmark::State& state) {
    int a = 0;
  for (auto _ : state) {
      auto* one = new int;
      *one = a++;
      benchmark::DoNotOptimize(one);
      delete one;
  }
}
BENCHMARK(NoTry);

// BENCHMARK_MAIN();
