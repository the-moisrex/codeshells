#include <benchmark/benchmark.h>
#include <vector>
#include <any>
#include <memory>

using namespace std;

template <typename T>
struct A {
	T value;
	A(T value) : value(std::move(value)) {}
};


static void EmptyRandom(benchmark::State& state) {

	std::vector<std::string> strs;
	for (int i = 0; i < 10000; i++)
		strs.emplace_back("value");

  for (auto _ : state) {
	for (auto &str : strs)
		if (str != "value")
			return;
    //benchmark::DoNotOptimize(c);
  }

}
// Register the function as a benchmark
BENCHMARK(EmptyRandom);


static void Templated(benchmark::State& state) {

	std::vector<A<std::string>> strs;
	for (int i = 0; i < 10000; i++)
		strs.push_back(A<std::string>{"value"});

  for (auto _ : state) {
	for (auto &str : strs)
		if (str.value != "value")
			return;
    //benchmark::DoNotOptimize(c);
  }

}
// Register the function as a benchmark
BENCHMARK(Templated);

static void Any(benchmark::State& state) {

	std::vector<any> strs;
	for (int i = 0; i < 10000; i++)
		strs.push_back(A<std::string>{"value"});

  for (auto _ : state) {
	for (auto &str : strs)
		if (any_cast<A<string>>(str).value != "value")
			return;
    //benchmark::DoNotOptimize(c);
  }

}
// Register the function as a benchmark
BENCHMARK(Any);




	template<typename T>
	T* my_any_cast (void*a) {
		return reinterpret_cast<T*>(a);
	}


static void MyAny(benchmark::State& state) {

	std::vector<void*> strs;
	for (int i = 0; i < 10000; i++)
		strs.push_back(
				new A<std::string>{"value"}
			);

  for (auto _ : state) {
	for (auto &str : strs)
		if (my_any_cast<A<string>>(str)->value != "value")
			return;
    //benchmark::DoNotOptimize(c);
  }

}
// Register the function as a benchmark
BENCHMARK(MyAny);

// BENCHMARK_MAIN();
