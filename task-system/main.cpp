#include "single_queue.h"
#include <benchmark/benchmark.h>
#include <iostream>
using namespace std;

static void V1(benchmark::State &state) {
  v1::task_system ts;
  for (auto _ : state) {
    ts.async_([&] { benchmark::DoNotOptimize(_); });
  }
}
BENCHMARK(V1);

static void V1Full(benchmark::State &state) {
  for (auto _ : state) {
    v1::task_system ts;
    for (int i = 0; i < 1000; ++i)
      ts.async_([&] { benchmark::DoNotOptimize(_); });
  }
}
BENCHMARK(V1Full);


static void V2(benchmark::State &state) {
  v2::task_system ts;
  for (auto _ : state) {
    ts.async_([&] { benchmark::DoNotOptimize(_); });
  }
}
BENCHMARK(V2);

static void V2Full(benchmark::State &state) {
  for (auto _ : state) {
    v2::task_system ts;
    for (int i = 0; i < 1000; ++i)
      ts.async_([&] { benchmark::DoNotOptimize(_); });
  }
}
BENCHMARK(V2Full);



static void V3(benchmark::State &state) {
  v3::task_system ts;
  for (auto _ : state) {
    ts.async_([&] { benchmark::DoNotOptimize(_); });
  }
}
BENCHMARK(V3);

static void V3Full(benchmark::State &state) {
  for (auto _ : state) {
    v3::task_system ts;
    for (int i = 0; i < 1000; ++i)
      ts.async_([&] { benchmark::DoNotOptimize(_); });
  }
}
BENCHMARK(V3Full);



