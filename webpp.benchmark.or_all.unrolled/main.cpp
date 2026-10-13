#include <benchmark/benchmark.h>

using namespace std;

// Micro-benchmark: scalar vs 4×-unrolled stop-token or_all.
//
// Drop into benchmarks/ (or build standalone against Google Benchmark).
// Intended filter:
//   ./build/url-bench-main --benchmark_filter='BM_OrAll' \
//       --benchmark_repetitions=5 --benchmark_min_time=0.5s
//
// Measures only the scanner; does not exercise parse_path / parse_host.
// Canary rows for the full URL bench remain mailto_opaque (opaque path).

#include <array>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// Minimal stand-in for webpp::categorize / path_interesting_chars shape.
// Bits mirror path_cp_type: stop_token=1, slash=0x21, termination=0x41, etc.
// ---------------------------------------------------------------------------

enum : std::uint8_t {
  k_normal = 0,
  k_stop_token = 0b1U,
  k_percent = 0b10U,
  k_dot = 0b100U,
  k_encoding = 0b1000U,
  k_slash = 0b10'0000U | k_stop_token,        // 0x21
  k_termination = 0b100'0000U | k_stop_token, // 0x41
};

[[nodiscard]] constexpr std::array<std::uint8_t, 256>
make_path_table() noexcept {
  std::array<std::uint8_t, 256> t{};
  t[static_cast<unsigned>('%')] = k_percent | k_stop_token;
  t[static_cast<unsigned>('.')] = k_dot;
  t[static_cast<unsigned>('/')] = k_slash;
  t[static_cast<unsigned>('\\')] = k_slash;
  t[static_cast<unsigned>('?')] = k_termination;
  t[static_cast<unsigned>('#')] = k_termination;
  // A few PATH_ENCODE_SET stand-ins (space, '"', '<', '>', etc.)
  t[static_cast<unsigned>(' ')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('"')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('<')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('>')] = k_encoding | k_stop_token;
  return t;
}

constexpr auto path_table = make_path_table();
constexpr std::uint8_t stop_token = k_stop_token;

// ---------------------------------------------------------------------------
// Scalar stop-token or_all (current production shape)
// ---------------------------------------------------------------------------

template <typename T, std::size_t N, typename Iter>
[[nodiscard]] constexpr T or_all_scalar(std::array<T, N> const &arr,
                                        T const stop, Iter &pos,
                                        Iter const end) noexcept {
  using char_type =
      std::make_unsigned_t<typename std::iterator_traits<Iter>::value_type>;
  constexpr auto last_el = static_cast<char_type>(N - 1U);
  T res{};
  for (; pos != end; ++pos) {
    auto const code = arr[static_cast<std::uint8_t>(
        std::min<char_type>(static_cast<char_type>(*pos), last_el))];
    res |= code;
    if ((code & stop) == stop) {
      break; // pos stays on the stop character
    }
  }
  return res;
}

// ---------------------------------------------------------------------------
// 4×-unrolled stop-token or_all (candidate)
// ---------------------------------------------------------------------------

template <typename T, std::size_t N, typename Iter>
[[nodiscard]] constexpr T or_all_unrolled(std::array<T, N> const &arr,
                                          T const stop, Iter &pos,
                                          Iter const end) noexcept {
  using char_type =
      std::make_unsigned_t<typename std::iterator_traits<Iter>::value_type>;
  constexpr auto last_el = static_cast<char_type>(N - 1U);
  T res{};

  auto const code_at = [&](Iter const it) noexcept {
    return arr[static_cast<std::uint8_t>(
        std::min<char_type>(static_cast<char_type>(*it), last_el))];
  };

  while (std::next(pos, 4) <= end) {
    auto const c0 = code_at(pos);
    if ((c0 & stop) == stop) {
      res |= c0;
      return res; // pos already at c0
    }

    auto const c1 = code_at(std::next(pos, 1));
    if ((c1 & stop) == stop) {
      res |= c0 | c1;
      pos = std::next(pos, 1);
      return res;
    }

    auto const c2 = code_at(std::next(pos, 2));
    if ((c2 & stop) == stop) {
      res |= c0 | c1 | c2;
      pos = std::next(pos, 2);
      return res;
    }

    auto const c3 = code_at(std::next(pos, 3));
    res |= c0 | c1 | c2 | c3;
    if ((c3 & stop) == stop) {
      pos = std::next(pos, 3);
      return res;
    }
    pos = std::next(pos, 4);
  }

  for (; pos != end; ++pos) {
    auto const code = code_at(pos);
    res |= code;
    if ((code & stop) == stop) {
      break;
    }
  }
  return res;
}

// ---------------------------------------------------------------------------
// Input fixtures (path-like / host-like)
// ---------------------------------------------------------------------------

struct Fixture {
  std::string data;
  char const *name;
};

// Long plain path segments (no stop until end) — best case for unroll.
std::string make_plain_path(std::size_t segs, std::size_t seg_len) {
  std::string s;
  s.reserve(segs * (seg_len + 1));
  for (std::size_t i = 0; i < segs; ++i) {
    s.push_back('/');
    for (std::size_t j = 0; j < seg_len; ++j) {
      s.push_back(static_cast<char>('a' + static_cast<int>((i + j) % 26)));
    }
  }
  return s;
}

// Path with a terminator at a chosen offset.
std::string make_path_with_stop(std::size_t plain_prefix, char stop_char) {
  std::string s;
  s.reserve(plain_prefix + 8);
  s.push_back('/');
  for (std::size_t i = 1; i < plain_prefix; ++i) {
    s.push_back(static_cast<char>('a' + static_cast<int>(i % 26)));
  }
  s.push_back(stop_char); // '?', '#', or '%'
  s.append("rest");
  return s;
}

std::vector<Fixture> const &fixtures() {
  static std::vector<Fixture> f = {
      {make_plain_path(32, 16), "plain_long_path"},      // ~544 bytes, no stop
      {make_plain_path(8, 8), "plain_medium_path"},      // ~72 bytes
      {make_plain_path(2, 4), "plain_short_path"},       // ~10 bytes
      {make_path_with_stop(4, '?'), "stop_early_query"}, // stop near start
      {make_path_with_stop(64, '?'), "stop_mid_query"},  // stop mid-run
      {make_path_with_stop(256, '#'), "stop_late_fragment"}, // stop late
      {make_path_with_stop(32, '%'), "stop_mid_percent"},
      {std::string(128, 'x') + "://host.example/path", "host_like_prefix"},
      {"/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p",
       "many_slashes"}, // '/' carries stop_token
  };
  return f;
}

// ---------------------------------------------------------------------------
// Correctness self-check (runs once at process start)
// ---------------------------------------------------------------------------

void assert_equivalent() {
  for (auto const &fx : fixtures()) {
    auto const *beg = fx.data.data();
    auto const *end = beg + fx.data.size();

    auto *p0 = beg;
    auto r0 = or_all_scalar(path_table, stop_token, p0, end);

    auto *p1 = beg;
    auto r1 = or_all_unrolled(path_table, stop_token, p1, end);

    if (r0 != r1 || p0 != p1) {
      std::fprintf(
          stderr,
          "or_all mismatch on fixture '%s': "
          "scalar(status=%u, off=%td) vs unrolled(status=%u, off=%td)\n",
          fx.name, static_cast<unsigned>(r0), p0 - beg,
          static_cast<unsigned>(r1), p1 - beg);
      std::abort();
    }
  }
}

struct Once {
  Once() { assert_equivalent(); }
};
Once const g_once{};

// ---------------------------------------------------------------------------
// Benchmarks
// ---------------------------------------------------------------------------

void BM_OrAll_Scalar(benchmark::State &state) {
  auto const &fx = fixtures()[static_cast<std::size_t>(state.range(0))];
  auto const *beg = fx.data.data();
  auto const *end = beg + fx.data.size();
  std::uint8_t sink = 0;

  for (auto _ : state) {
    auto *pos = beg;
    auto st = or_all_scalar(path_table, stop_token, pos, end);
    sink ^= static_cast<std::uint8_t>(st);
    sink ^= static_cast<std::uint8_t>(pos - beg);
    benchmark::DoNotOptimize(sink);
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(static_cast<std::int64_t>(state.iterations()) *
                          static_cast<std::int64_t>(fx.data.size()));
  state.SetLabel(fx.name);
}

void BM_OrAll_Unrolled(benchmark::State &state) {
  auto const &fx = fixtures()[static_cast<std::size_t>(state.range(0))];
  auto const *beg = fx.data.data();
  auto const *end = beg + fx.data.size();
  std::uint8_t sink = 0;

  for (auto _ : state) {
    auto *pos = beg;
    auto st = or_all_unrolled(path_table, stop_token, pos, end);
    sink ^= static_cast<std::uint8_t>(st);
    sink ^= static_cast<std::uint8_t>(pos - beg);
    benchmark::DoNotOptimize(sink);
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(static_cast<std::int64_t>(state.iterations()) *
                          static_cast<std::int64_t>(fx.data.size()));
  state.SetLabel(fx.name);
}

BENCHMARK(BM_OrAll_Scalar)->DenseRange(0, 8)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_OrAll_Unrolled)->DenseRange(0, 8)->Unit(benchmark::kNanosecond);

} // namespace

// BENCHMARK_MAIN();
