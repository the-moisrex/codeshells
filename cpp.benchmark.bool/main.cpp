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

int test4(char32_t cu2, char32_t code_point, char32_t cu3) {
  code_point |= (cu2 & 0b0000'1111) << 12U;
  code_point |= (cu3 & 0b0011'1111) << 6U;
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

int test5(char32_t cu2, char32_t code_point, char32_t cu3) {
  code_point |= (cu2 & 0b0000'1111) << 12U;
  code_point |= (cu3 & 0b0011'1111) << 6U;
  if ((cu2 & 0b1111'0000) != 0b1110'0000 || code_point < 0x800 ||
      0xFFFF < code_point || (0xD7FF < code_point && code_point < 0xE000))
      [[unlikely]] {
    return 1;
  }
  return 2;
}

volatile char32_t one = 1, two = 2, three = 3;

static void One(benchmark::State &state) {
  for (auto _ : state) {
    auto res = test1(one, two);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(One);

static void Two(benchmark::State &state) {
  for (auto _ : state) {
    auto res = test2(one, two);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(Two);

static void Three(benchmark::State &state) {
  for (auto _ : state) {
    auto res = test3(one, two);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(Three);

static void Four(benchmark::State &state) {
  for (auto _ : state) {
    auto res = test4(one, two, three);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(Four);

static void Five(benchmark::State &state) {
  for (auto _ : state) {
    auto res = test5(one, two, three);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(Five);
