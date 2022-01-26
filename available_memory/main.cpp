#include <benchmark/benchmark.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <chrono>
#include <utility>
#include <type_traits>

using namespace std;


auto pagesize = sysconf(_SC_PAGESIZE);


static void Zero(benchmark::State& state) {

  for (auto _ : state) {
    benchmark::DoNotOptimize(1);
  }

}
// Register the function as a benchmark
BENCHMARK(Zero);




static void One(benchmark::State& state) {

  for (auto _ : state) {
	auto c = sysconf(_SC_AVPHYS_PAGES) * pagesize;
    benchmark::DoNotOptimize(c);
  }

}
// Register the function as a benchmark
BENCHMARK(One);





static void Two(benchmark::State& state) {

  for (auto _ : state) {
	auto c = get_avphys_pages() * pagesize;
    benchmark::DoNotOptimize(c);
  }

}
// Register the function as a benchmark
BENCHMARK(Two);



auto getAv() {
	using namespace std::chrono;

	static long c = 0;
	static auto t = steady_clock::now();
	if (c == 0 || (steady_clock::now() - t).count() > 1000) {
		c = get_avphys_pages() * pagesize;
		t = steady_clock::now();
	}
	return c;
	
}


auto getAv2() {
	using namespace std::chrono;

	static long c = 0;
	static auto t = system_clock::now();
	if (c == 0 || (system_clock::now() - t).count() > 1000) {
		c = get_avphys_pages() * pagesize;
		t = system_clock::now();
	}
	return c;
	
}


template <typename Callable>
auto bounced(Callable const& callable, unsigned long long interval = 1000) {
	using namespace std::chrono;

	static_assert(std::is_invocable_v<Callable>, "The specified callable is not callable");

	constexpr bool does_returns_void = std::is_void_v<decltype(callable())>;
	static time_point<steady_clock> t;

	if constexpr (!does_returns_void) {
		static decltype(callable()) res;
		if ((steady_clock::now() - t).count() > interval) {
			res = callable();
			t = steady_clock::now();
			
		}
		return res;
	} else {
		if ((steady_clock::now() - t).count() > interval) {
			callable();
			t = steady_clock::now();
			
		}
	}
}


static void Three(benchmark::State& state) {
  for (auto _ : state) {
	  auto c = getAv();
    benchmark::DoNotOptimize(c);
  }

}
BENCHMARK(Three);





static void Four(benchmark::State& state) {
  for (auto _ : state) {
	  auto c = getAv2();
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(Four);






static void Five(benchmark::State& state) {
  for (auto _ : state) {
	  auto c = bounced(&getAv);
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(Five);

// BENCHMARK_MAIN();
