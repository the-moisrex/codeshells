#include <benchmark/benchmark.h>


int test1(char32_t cu2, char32_t code_point) {
    if ((cu2 & 0b1111'0000) != 0b1110'0000 || code_point < 0x800 ||
        0xFFFF < code_point || (0xD7FF < code_point && code_point < 0xE000))
        [[unlikely]] {
            return 1;
    }
    return 2;
}


int test2(char32_t cu2, char32_t code_point) {
    bool error = false;
    error |= (cu2 & 0b1111'0000) != 0b1110'0000;
    error |= code_point < 0x800;
    error |= 0xFFFF < code_point;
    error |= (0xD7FF < code_point && code_point < 0xE000);
    if (error) [[unlikely]] {
        return 1;
    }
    return 2;
}

int test3(char32_t cu2, char32_t code_point) {
    bool error = false;
    error |= (cu2 >> 4U) != 0b1110;
    error |= code_point < 0x800;
    error |= 0xFFFF < code_point;
    error |= (0xD7FF < code_point && code_point < 0xE000);
    if (error) [[unlikely]] {
        return 1;
    }
    return 2;
}


static void One(benchmark::State& state) {
  for (auto _ : state) {
    auto res = test1(1, 2);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(One);



static void Two(benchmark::State& state) {
  for (auto _ : state) {
    auto res = test2(1, 2);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(Two);

static void Three(benchmark::State& state) {
  for (auto _ : state) {
    auto res = test3(1, 2);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(Three);

