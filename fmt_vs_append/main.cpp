#include <benchmark/benchmark.h>
#include <fmt/format.h>
#include <string>

using namespace std;

namespace base {
template <typename Container>
constexpr auto size(const Container& c) -> decltype(c.size()) {
  return c.size();
}

template <typename T, size_t N>
constexpr size_t size(const T (&array)[N]) noexcept {
  return N;
}
}



static void Append(benchmark::State& state) {
  const string request_method_ = "GET";
  const std::string path = "/home/moisrex";
  for (auto _ : state) {
    static const char kSuffix[] = " HTTP/1.1\r\n";
    const size_t kSuffixLen = base::size(kSuffix) - 1;
    std::string request_line;
    const size_t expected_size =
      request_method_.size() + 1 + path.size() + kSuffixLen;
    request_line.reserve(expected_size);
    request_line.append(request_method_);
    request_line.append(1, ' ');
    request_line.append(path);
    request_line.append(kSuffix, kSuffixLen);
    benchmark::DoNotOptimize(request_line);
  }
}
BENCHMARK(Append);

static void FMT(benchmark::State& state) {
  const string request_method_ = "GET";
  const std::string path = "/home/moisrex";
  for (auto _ : state) {
    std::string request_line = fmt::format("%s %s HTTP/1.1\r\n", request_method_, path);
    benchmark::DoNotOptimize(request_line);
  }
}
BENCHMARK(FMT);


// BENCHMARK_MAIN();
