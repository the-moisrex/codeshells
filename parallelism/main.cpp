#include <benchmark/benchmark.h>
#include <vector>
#include <execution>
#include <algorithm>
#include <random>
#include <functional>


using namespace std;

vector<int> vec() {
	static vector<int> ints;
	if (ints.empty()) {
		mt19937 rng;
		generate_n(execution::par, std::back_inserter(ints), 1000, ref(rng));
	}

	// copy the data
	return ints;
}

static void EmptyRandom(benchmark::State& state) {
  for (auto _ : state) {
    // benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(EmptyRandom);






static void simple(benchmark::State& state) {
	auto ints = vec();
  for (auto _ : state) {
	sort(ints.begin(), ints.end());
    // benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(simple);





static void psimple(benchmark::State& state) {
	auto ints = vec();
  for (auto _ : state) {
	sort(execution::par, ints.begin(), ints.end());
    // benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(psimple);


