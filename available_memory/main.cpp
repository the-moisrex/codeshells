#include <benchmark/benchmark.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <chrono>

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


// BENCHMARK_MAIN();
