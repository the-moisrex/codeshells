#include <benchmark/benchmark.h>
#include <vector>
#include <memory_resource>
#include <alloca.h>

#define VALUES 1,2,3,4,5,6,7,8,9,10,11,12,12,14,432,542,54,2354,32,5432,5432,5423,5432,543,254,3254,325,432,54,3254,325,432,5432,54,3254,32,6,876,5876587,76,876,587,658,765,54653,654,365,436,5786,65,7658,765,876,65436,54,365,4365,32542,45,675876876,56,55436,5432,65376,5487,6598,765875, 4763576,548,765,8765,8765,8754,765,487658,765,876,5876,5876,587,65876,4867,548765,987,6985876,48,6754,876,87,68,548,654,8654,8765,8764,85486,548764,876,58746,876,8765,8765876,5876,587658,765876,587658,7658,564654,54325,432,5,432,54,325,432,543,254,32,6554,7654,76,547,654,765,476,547,654,76,5476,54,76,547,654,76,547,654,76,547,6,547,654,76,547,654,76,547,654,763,54236452,56432,542,354,3254,32,45,675,87,6587658,765,7647,654,654,365,4365,436,543,65,436,543,65,436,54,36,543,6,5436,54,365,436,543,65,43,654,36,543,65,3465436,543,254,3254,325,432,54,3254,325,432,5432,54,3254,32,6,876,5876587,76,876,587,658,765,54653,654,365,436,5786,65,7658,765,876,65436,54,365,4365,32542,45,675876876,56,55436,5432,65376,5487,6598,765875, 4763576,548,765,8765,8765,8754,765,487658,765,876,5876,5876,587,65876,4867,548765,987,6985876,48,6754,876,87,68,548,654,8654,8765,8764,85486,548764,876,58746,876,8765,8765876,5876,587658,765876,587658,7658,564654,54325,432,5,432,54,325,432,543,254,32,6554,7654,76,547,654,765,476,547,654,76,5476,54,76,547,654,76,547,654,76,547,6,547,654,76,547,654,76,547,654,763,54236452,56432,542,354,3254,32,45,675,87,6587658,765,7647,654,654,365,4365,436,543,65,436,543,65,436,54,36,543,6,5436,54,365,436,543,65,43,654,36,543,65,3465436,543,254,3254,325,432,54,3254,325,432,5432,54,3254,32,6,876,5876587,76,876,587,658,765,54653,654,365,436,5786,65,7658,765,876,65436,54,365,4365,32542,45,675876876,56,55436,5432,65376,5487,6598,765875, 4763576,548,765,8765,8765,8754,765,487658,765,876,5876,5876,587,65876,4867,548765,987,6985876,48,6754,876,87,68,548,654,8654,8765,8764,85486,548764,876,58746,876,8765,8765876,5876,587658,765876,587658,7658,564654,54325,432,5,432,54,325,432,543,254,32,6554,7654,76,547,654,765,476,547,654,76,5476,54,76,547,654,76,547,654,76,547,6,547,654,76,547,654,76,547,654,763,54236452,56432,542,354,3254,32,45,675,87,6587658,765,7647,654,654,365,4365,436,543,65,436,543,65,436,54,36,543,6,5436,54,365,436,543,65,43,654,36,543,65,3465436,543,254,3254,325,432,54,3254,325,432,5432,54,3254,32,6,876,5876587,76,876,587,658,765,54653,654,365,436,5786,65,7658,765,876,65436,54,365,4365,32542,45,675876876,56,55436,5432,65376,5487,6598,765875, 4763576,548,765,8765,8765,8754,765,487658,765,876,5876,5876,587,65876,4867,548765,987,6985876,48,6754,876,87,68,548,654,8654,8765,8764,85486,548764,876,58746,876,8765,8765876,5876,587658,765876,587658,7658,564654,54325,432,5,432,54,325,432,543,254,32,6554,7654,76,547,654,765,476,547,654,76,5476,54,76,547,654,76,547,654,76,547,6,547,654,76,547,654,76,547,654,763,54236452,56432,542,354,3254,32,45,675,87,6587658,765,7647,654,654,365,4365,436,543,65,436,543,65,436,54,36,543,6,5436,54,365,436,543,65,43,654,36,543,65,3465436,543,254,3254,325,432,54,3254,325,432,5432,54,3254,32,6,876,5876587,76,876,587,658,765,54653,654,365,436,5786,65,7658,765,876,65436,54,365,4365,32542,45,675876876,56,55436,5432,65376,5487,6598,765875, 4763576,548,765,8765,8765,8754,765,487658,765,876,5876,5876,587,65876,4867,548765,987,6985876,48,6754,876,87,68,548,654,8654,8765,8764,85486,548764,876,58746,876,8765,8765876,5876,587658,765876,587658,7658,564654,54325,432,5,432,54,325,432,543,254,32,6554,7654,76,547,654,765,476,547,654,76,5476,54,76,547,654,76,547,654,76,547,6,547,654,76,547,654,76,547,654,763,54236452,56432,542,354,3254,32,45,675,87,6587658,765,7647,654,654,365,4365,436,543,65,436,543,65,436,54,36,543,6,5436,54,365,436,543,65,43,654,36,543,65,3465436,543


static void PMRWinkOutVectorNoTryCatch(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::array<std::uint8_t, 1024> buffer{};
  for (auto _ : state) {
    std::pmr::monotonic_buffer_resource underlying_bytes(buffer.data(),
                                                       buffer.size());
    std::pmr::polymorphic_allocator<> alloc{&underlying_bytes};
    auto *vec = alloc.allocate_object<std::pmr::vector<int>>();
    alloc.construct(vec, std::initializer_list<int>{VALUES});
    benchmark::DoNotOptimize(vec);
  }
}
// Register the function as a benchmark
BENCHMARK(PMRWinkOutVectorNoTryCatch);

static void PMRWinkOutVector(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::array<std::uint8_t, 1024> buffer{};
  for (auto _ : state) {
    std::pmr::monotonic_buffer_resource underlying_bytes(buffer.data(),
                                                       buffer.size());
    std::pmr::polymorphic_allocator<> alloc{&underlying_bytes};
    auto *vec = alloc.new_object<std::pmr::vector<int>>(
      std::initializer_list<int>{VALUES});
    benchmark::DoNotOptimize(vec);
  }
}
// Register the function as a benchmark
BENCHMARK(PMRWinkOutVector);


static void PMRVector(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::array<std::uint8_t, 1024> buffer{};
  for (auto _ : state) {
    std::pmr::monotonic_buffer_resource underlying_bytes(buffer.data(),
                                                       buffer.size());
    std::pmr::vector<int> vec({VALUES}, &underlying_bytes);
    benchmark::DoNotOptimize(vec);
  }
}
// Register the function as a benchmark
BENCHMARK(PMRVector);

static void PMRVectorAlloca(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  auto buffer = alloca(1024);
  for (auto _ : state) {
    std::pmr::monotonic_buffer_resource underlying_bytes(buffer,
                                                         1024);
    std::pmr::vector<int> vec({VALUES}, &underlying_bytes);
    benchmark::DoNotOptimize(vec);
  }
}
BENCHMARK(PMRVectorAlloca);

static void PMRVectorDynamic(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  auto buffer = malloc(1024);
  for (auto _ : state) {
    std::pmr::monotonic_buffer_resource underlying_bytes(buffer,
                                                         1024);
    std::pmr::vector<int> vec({VALUES}, &underlying_bytes);
    benchmark::DoNotOptimize(vec);
  }
  free(buffer);
}
BENCHMARK(PMRVectorDynamic);

static void Vector(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<int> vec({VALUES});
    benchmark::DoNotOptimize(vec);
  }
}
BENCHMARK(Vector);

