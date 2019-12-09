#include <benchmark/benchmark.h>
#include <functional>
#include <random>

using namespace std;

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> random_pick(0,1); // distribution in range [1, 6]

template <std::size_t N>
std::array<bool, N> get_random_array() {
  std::array<bool, N> item;
  for (int i = 0 ; i < N; i++)
    item[i] = static_cast<bool>(random_pick(rng));
  return item;
}

template <typename T, std::size_t N>
std::array<T, N> get_random_objects(std::function<T(decltype(random_pick(rng)))> func) {
    std::array<T, N> a;
    std::generate(a.begin(), a.end(), [&] {
        return func(random_pick(rng));
    });
    return a;
}


static void FuncWay(benchmark::State& state) {

	function<bool()> func = [] { return true; };
	auto arr = get_random_array<100>;
	auto i = 0;

  for (auto _ : state) {
	  bool rand_bool = arr[i++ % 100];
	  func = [=] () -> bool {
		return rand_bool && func();
	  };
	  auto a = func();
    benchmark::DoNotOptimize(func);
    benchmark::DoNotOptimize(a);
  }

}
// Register the function as a benchmark
BENCHMARK(FuncWay);



static void FuncRefWay(benchmark::State& state) {

	function<bool()> func = [] { return true; };
	auto arr = get_random_array<100>;
	auto i = 0;

  for (auto _ : state) {
	  bool rand_bool = arr[i++ % 100];
	auto aa = [=] () -> bool {
		return rand_bool && func();
	  };
	  func = std::ref(aa);
	  auto a = func();
    benchmark::DoNotOptimize(func);
    benchmark::DoNotOptimize(a);
  }

}
// Register the function as a benchmark
BENCHMARK(FuncRefWay);


// BENCHMARK_MAIN();
