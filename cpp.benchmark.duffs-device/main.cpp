#include <algorithm>
#include <benchmark/benchmark.h>

using namespace std;

// clang-format off
void copy_duff_style(char *to, char *from, int count) {
    int n = (count + 7) / 8;
    switch (count % 8) {
    case 0: do { *to++ = *from++;
    case 7:      *to++ = *from++;
    case 6:      *to++ = *from++;
    case 5:      *to++ = *from++;
    case 4:      *to++ = *from++;
    case 3:      *to++ = *from++;
    case 2:      *to++ = *from++;
    case 1:      *to++ = *from++;
            } while (--n > 0);
    }
}
// clang-format on

void simple_copy_n(char *from, int count, char *to) {
  auto end = from + count;
  for (; from != end;) {
    *to++ = *from++;
  }
}

static void EmptyRandom(benchmark::State &state) {
  for (auto _ : state) {
    // benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(EmptyRandom);

static void DuffsDevice(benchmark::State &state) {
  std::string str1, str2;
  str1.resize(1000);
  str2.resize(1000);
  for (auto _ : state) {
    copy_duff_style(str2.data(), str1.data(), str1.size());
    benchmark::DoNotOptimize(str1);
    benchmark::DoNotOptimize(str2);
  }
}
BENCHMARK(DuffsDevice);

static void StandardCopy(benchmark::State &state) {
  std::string str1, str2;
  str1.resize(1000);
  str2.resize(1000);
  for (auto _ : state) {
    std::copy_n(str1.data(), str1.size(), str2.data());
    benchmark::DoNotOptimize(str1);
    benchmark::DoNotOptimize(str2);
  }
}
BENCHMARK(StandardCopy);

static void SimpleCopying(benchmark::State &state) {
  std::string str1, str2;
  str1.resize(1000);
  str2.resize(1000);
  for (auto _ : state) {
    simple_copy_n(str1.data(), str1.size(), str2.data());
    benchmark::DoNotOptimize(str1);
    benchmark::DoNotOptimize(str2);
  }
}
BENCHMARK(SimpleCopying);
