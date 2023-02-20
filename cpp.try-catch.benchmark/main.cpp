#include <benchmark/benchmark.h>
#include <exception>

using namespace std;

static void without_exception(benchmark::State &state) {
  for (auto _ : state) {
    std::vector<uint32_t> v(10000);
    for (uint32_t i = 0; i < 10000; i++) {
      v.at(i) = i;
      benchmark::DoNotOptimize(v.at(i));
    }
  }
}
BENCHMARK(without_exception); //----------------------------------------

static void with_exception(benchmark::State &state) {
  for (auto _ : state) {
    std::vector<uint32_t> v(10000);
    for (uint32_t i = 0; i < 10000; i++) {
      try {
        v.at(i) = i;
        benchmark::DoNotOptimize(v.at(i));
      } catch (const std::out_of_range &oor) {
      }
    }
  }
}
BENCHMARK(with_exception); //--------------------------------------------

static void throwing_exception(benchmark::State &state) {
  for (auto _ : state) {
    std::vector<uint32_t> v(10000);
    for (uint32_t i = 1; i < 10001; i++) {
      try {
        v.at(i) = i;
        benchmark::DoNotOptimize(v.at(i));
      } catch (const std::out_of_range &oor) {
      }
    }
  }
}
BENCHMARK(throwing_exception); //-----------------------------------------
