#include <benchmark/benchmark.h>
#include <array>

using namespace std;
template <std::size_t ArraySize = 100>
constexpr auto create_array(const int multiplier) {
  std::array<int, ArraySize> result{};
  int counter = 0;
  for (int &i : result) 
      i = multiplier*(++counter);
  return result;
}

static void Static(benchmark::State& state) {
  for (auto _ : state) {
    static constexpr auto my_array = create_array(12);
    benchmark::DoNotOptimize(my_array);
  }
}
BENCHMARK(Static);

static void NoStatic(benchmark::State& state) {
  for (auto _ : state) {
    constexpr auto my_array = create_array(12);
    benchmark::DoNotOptimize(my_array);
  }
}
BENCHMARK(NoStatic);


static void ConstInit(benchmark::State& state) {
  for (auto _ : state) {
    static constinit auto my_array = create_array(12);
    benchmark::DoNotOptimize(my_array);
  }
}
BENCHMARK(ConstInit);




