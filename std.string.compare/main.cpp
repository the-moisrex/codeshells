#include <benchmark/benchmark.h>
#include <vector>
#include <string>
#include <random>

using namespace std;
std::string random_string(std::string::size_type length)
{
    static auto& chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    static std::mt19937 rg{std::random_device{}()};
    static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;

    s.reserve(length);

    while(length--)
        s += chrs[pick(rg)];

    return s;
}


auto get_rand_strs() {
  static vector<string> r;
  if (r.empty()) {
    for (int i = 0; i < 1000; i++) {
      r.push_back(random_string(i));
    }
  }
  return r;
}


auto strs = get_rand_strs();

auto get_str(vector<string> const& s) {
  static int i = 0;
  return s[i++ % s.size()];
}

static void STD(benchmark::State& state) {
  for (auto _ : state) {
    auto one = get_str(strs);
    auto two = get_str(strs);
    benchmark::DoNotOptimize(one == two);
  }

}
BENCHMARK(STD);


static void MySTD(benchmark::State& state) {
  for (auto _ : state) {
    auto one = get_str(strs);
    auto two = get_str(strs);
    benchmark::DoNotOptimize(one.size() == two.size() and one == two);
  }

}
BENCHMARK(MySTD);

// BENCHMARK_MAIN();
