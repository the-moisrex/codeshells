#include <benchmark/benchmark.h>
#include <string_view>

using namespace std;

static constexpr std::string_view progress_bar = "🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵⚪⚪⚪⚪⚪⚪⚪⚪⚪⚪";
constexpr std::string_view table[] = {"⚪⚪⚪⚪⚪⚪⚪⚪⚪",  "🔵⚪⚪⚪⚪⚪⚪⚪⚪",  "🔵🔵⚪⚪⚪⚪⚪⚪⚪",
                                      "🔵🔵🔵⚪⚪⚪⚪⚪⚪",  "🔵🔵🔵🔵⚪⚪⚪⚪⚪",  "🔵🔵🔵🔵🔵⚪⚪⚪⚪⚪",
                                      "🔵🔵🔵🔵🔵🔵⚪⚪⚪⚪", "🔵🔵🔵🔵🔵🔵🔵⚪⚪⚪", "🔵🔵🔵🔵🔵🔵🔵🔵⚪⚪",
                                      "🔵🔵🔵🔵🔵🔵🔵🔵🔵⚪", "🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵"};

static constexpr std::string_view getPercentageRounds(double percentage) noexcept {
  auto const blue = std::min<std::size_t>(percentage * 10, 10);
  return progress_bar.substr(40 - 4 * blue, 30 + blue);
}

static constexpr std::string_view progress(double percentage) noexcept {
  return table[std::clamp<std::size_t>(
      static_cast<std::size_t>(std::max(percentage, 0.0) / 10), 0, 10)];
}

static void EmptyRandom(benchmark::State &state) {
  double index = 0;
  for (auto _ : state) {
    index += 0.1;
    if (index == 1.0)
      index = 0;
    benchmark::DoNotOptimize(index);
  }
}
BENCHMARK(EmptyRandom);

static void One(benchmark::State &state) {
  double index = 0;
  for (auto _ : state) {
    index += 0.1;
    if (index == 1.0)
      index = 0;
    const auto prog = getPercentageRounds(index);
    benchmark::DoNotOptimize(prog);
  }
}
BENCHMARK(One);

static void Two(benchmark::State &state) {
  double index = 0;
  for (auto _ : state) {
    index += 0.1;
    if (index == 1.0)
      index = 0;
    const auto prog = progress(index);
    benchmark::DoNotOptimize(prog);
  }
}
BENCHMARK(Two);
