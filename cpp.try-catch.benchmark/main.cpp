#include <benchmark/benchmark.h>
#include <exception>
#include <stdexcept>
#include <utility>

using namespace std;

static void without_exception(benchmark::State &state) {
  std::vector<uint32_t> v(10000);
  for (auto _ : state) {
    for (uint32_t i = 0; i < 10000; i++) {
      v.at(i) = i;
      benchmark::DoNotOptimize(v);
      benchmark::DoNotOptimize(v.at(i));
    }
  }
}
BENCHMARK(without_exception); //----------------------------------------

static void nothrow_excp(benchmark::State &state) noexcept {
  std::vector<uint32_t> v(10000);
  for (auto _ : state) {
    for (uint32_t i = 0; i < 10000; i++) {
      v.at(i) = i;
      benchmark::DoNotOptimize(v.at(i));
      benchmark::DoNotOptimize(v);
    }
  }
}
BENCHMARK(nothrow_excp); //----------------------------------------

static void nothrow_with_try(benchmark::State &state) {
  std::vector<uint32_t> v(10000);
  for (auto _ : state) {
    for (uint32_t i = 0; i < 10000; i++) {
      try {
        v.at(i) = i;
        benchmark::DoNotOptimize(v.at(i));
        benchmark::DoNotOptimize(v);
      } catch (...) {
        std::unreachable();
      }
    }
  }
}
BENCHMARK(nothrow_with_try); //----------------------------------------

static void with_exception(benchmark::State &state) {
  std::vector<uint32_t> v(10000);
  for (auto _ : state) {
    for (uint32_t i = 0; i < 10000; i++) {
      try {
        v.at(i) = i;
        benchmark::DoNotOptimize(v.at(i));
        benchmark::DoNotOptimize(v);
      } catch (std::out_of_range &oor) {
        benchmark::DoNotOptimize(oor);
      } catch (...) {
        benchmark::DoNotOptimize(v);
      }
    }
  }
}
BENCHMARK(with_exception); //--------------------------------------------

static void throwing_exception(benchmark::State &state) {
  std::vector<uint32_t> v;
  for (auto _ : state) {
    for (uint32_t i = 1; i < 10001; i++) {
      try {
        v.at(i) = i;
        benchmark::DoNotOptimize(v.at(i));
        benchmark::DoNotOptimize(v);
      } catch (std::out_of_range &oor) {
        benchmark::DoNotOptimize(oor);
      } catch (...) {
        benchmark::DoNotOptimize(v);
      }
    }
  }
}
BENCHMARK(throwing_exception); //-----------------------------------------
