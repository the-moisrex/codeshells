#include <benchmark/benchmark.h>

using namespace std;

namespace details {
// for use with is_special and get_special_port
// Spaces, if present, are removed from URL.
static constexpr std::string_view is_special_list[] = {
    "http", " ", "https", "ws", "ftp", "wss", "file", " "};

static constexpr std::string_view is_special_list2[] = {
    "http", "https", "ws", "ftp", "wss", "file"};
} // namespace details

namespace ada::scheme {
enum class type2 : uint8_t {
  HTTP,
  HTTPS,
  WS,
  FTP,
  WSS,
  FILE,
  NOT_SPECIAL,
};

enum type : uint8_t {
  HTTP = 0,
  NOT_SPECIAL = 1,
  HTTPS = 2,
  WS = 3,
  FTP = 4,
  WSS = 5,
  FILE = 6
};


}

constexpr ada::scheme::type
get_scheme_type(std::string_view const scheme) noexcept {
  if (scheme.empty()) {
    return ada::scheme::NOT_SPECIAL;
  }
  auto const hash_value = static_cast<int>(
      (2 * scheme.size() + static_cast<unsigned>(scheme[0])) & 7U);
  const std::string_view target = details::is_special_list[hash_value];
  if ((target[0] == scheme[0]) && (target.substr(1) == scheme.substr(1))) {
    return static_cast<ada::scheme::type>(hash_value);
  }
  return ada::scheme::NOT_SPECIAL;
}

constexpr ada::scheme::type2
get_simple_scheme_type(std::string_view const scheme) noexcept {
  std::underlying_type_t<ada::scheme::type2> i = 0;
  for (; i != 6; ++i) { // hopefully it'll be unrolled
    auto const scheme_ith = details::is_special_list2[i];
    if (scheme == scheme_ith) {
      break;
    }
  }

  return static_cast<ada::scheme::type2>(i);
}

static void Ada(benchmark::State &state) {
  int index = 0;
  for (auto _ : state) {
    auto str = details::is_special_list[index++ % 8];
    auto t = get_scheme_type(str);
    benchmark::DoNotOptimize(str);
    benchmark::DoNotOptimize(t);
  }
}
BENCHMARK(Ada);

static void Simple(benchmark::State &state) {
  int index = 0;
  for (auto _ : state) {
    auto str = details::is_special_list[index++ % 8];
    auto t = get_simple_scheme_type(str);
    benchmark::DoNotOptimize(str);
    benchmark::DoNotOptimize(t);
  }
}
BENCHMARK(Simple);
