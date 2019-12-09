#include <benchmark/benchmark.h>
#include <string>

using namespace std;

static void str(benchmark::State& state) {

	std::string a = "fdkhgfkdshgkfdhsgklfdshgklfdhsglkfdhsgfdsgkfdhsgfdkl"
		"fgdkjsghfdkshgfkahfdslkafhdkslafdhlskajfhdlsakjfhdlksajf"
		"dfskghfdkjsghfdkshgfdkslghfdlksghfdksghfdlskjgfds"
		"dfskjghfdskghfddfjshgfkldshgkfldshgklfdhsgklfdhsgklfdjshglkfd"
		"jhfdkgjhfdkjshgfjkdhgklfdshgklfdhgkfdsjhgklfdshgdklsghfdslk";


  for (auto _ : state) {

	void* b= new std::string(std::move(a));

    benchmark::DoNotOptimize(b);
    	delete static_cast<std::string*>(b);
  }

}
// Register the function as a benchmark
BENCHMARK(str);


// BENCHMARK_MAIN();
