#include <benchmark/benchmark.h>
#include <memory_resource>
#include <string>
#include <vector>
#include <array>
#include <alloca.h>

using namespace std;

static void PMR(benchmark::State& state) {
  for (auto _ : state) {
    array<char, 1024> buffer;
    pmr::monotonic_buffer_resource mr(buffer.data(), buffer.size());
    pmr::polymorphic_allocator<> p{&mr};
    pmr::vector<pmr::string> strs{p};
    for (int i = 0; i < 100; i++) {
      array<char, 2> str{static_cast<char>(i), 0};
      strs.emplace_back(str.data(), 1);
    }
    benchmark::DoNotOptimize(strs);
  }
}
BENCHMARK(PMR);

static void PMRNoBuffer(benchmark::State& state) {
  for (auto _ : state) {
    array<char, 1024> buffer;
    pmr::monotonic_buffer_resource mr(buffer.data(), buffer.size());
    pmr::vector<pmr::string> strs{&mr};
    for (int i = 0; i < 100; i++) {
      array<char, 2> str{static_cast<char>(i), 0};
      strs.emplace_back(str.data(), 1);
    }
    benchmark::DoNotOptimize(strs);
  }
}
BENCHMARK(PMRNoBuffer);



static void PMRStack(benchmark::State& state) {
  void* buffer = alloca(1024);
  for (auto _ : state) {
    pmr::monotonic_buffer_resource mr(buffer, 1024);
    pmr::vector<pmr::string> strs{&mr};
    for (int i = 0; i < 100; i++) {
      array<char, 2> str{static_cast<char>(i), 0};
      strs.emplace_back(str.data(), 1);
    }
    benchmark::DoNotOptimize(strs);
  }
}
BENCHMARK(PMRStack);


static void PMRDynamic(benchmark::State& state) {
  for (auto _ : state) {
    void* buffer = malloc(1024);
    pmr::monotonic_buffer_resource mr(buffer, 1024);
    pmr::vector<pmr::string> strs{&mr};
    for (int i = 0; i < 100; i++) {
      array<char, 2> str{static_cast<char>(i), 0};
      strs.emplace_back(str.data(), 1);
    }
    benchmark::DoNotOptimize(strs);
  }
}
BENCHMARK(PMRDynamic);


static void PMRDynamic2(benchmark::State& state) {
  void* buffer = malloc(1024);
  for (auto _ : state) {
    pmr::monotonic_buffer_resource mr(buffer, 1024);
    pmr::vector<pmr::string> strs{&mr};
    for (int i = 0; i < 100; i++) {
      array<char, 2> str{static_cast<char>(i), 0};
      strs.emplace_back(str.data(), 1);
    }
    benchmark::DoNotOptimize(strs);
  }
}
BENCHMARK(PMRDynamic2);



