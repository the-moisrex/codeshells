// Micro-benchmark: scalar vs 4×-unrolled stop-token or_all (fixed fixtures).
//
// Critical fix vs the previous version:
//   - Separate tables: path-segment ( '/' stops ) vs bulk-region ( '/' does NOT
//   stop ).
//   - Inputs that actually exercise long runs.
//   - Stronger DoNotOptimize so LTO cannot fold the scan away.

#include <array>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>

namespace {

enum : std::uint8_t {
  k_stop_token = 0b1U,
  k_percent = 0b10U,
  k_dot = 0b100U,
  k_encoding = 0b1000U,
  k_slash_only = 0b10'0000U,                // no stop bit — region-scan style
  k_slash_stop = 0b10'0000U | k_stop_token, // segment-loop style
  k_termination = 0b100'0000U | k_stop_token,
};

// Segment-loop table: '/' and '\' carry stop_token (matches
// path_interesting_chars).
[[nodiscard]] constexpr std::array<std::uint8_t, 256>
make_segment_table() noexcept {
  std::array<std::uint8_t, 256> t{};
  t[static_cast<unsigned>('%')] = k_percent | k_stop_token;
  t[static_cast<unsigned>('.')] = k_dot;
  t[static_cast<unsigned>('/')] = k_slash_stop;
  t[static_cast<unsigned>('\\')] = k_slash_stop;
  t[static_cast<unsigned>('?')] = k_termination;
  t[static_cast<unsigned>('#')] = k_termination;
  t[static_cast<unsigned>(' ')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('"')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('<')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('>')] = k_encoding | k_stop_token;
  return t;
}

// Region-scan table: '/' does NOT stop (stop_bits = mask & ~stop_token).
[[nodiscard]] constexpr std::array<std::uint8_t, 256>
make_region_table() noexcept {
  std::array<std::uint8_t, 256> t{};
  t[static_cast<unsigned>('%')] = k_percent | k_stop_token;
  t[static_cast<unsigned>('.')] =
      k_dot | k_stop_token; // treat dot as region end for this microbench
  t[static_cast<unsigned>('/')] =
      k_slash_only; // no stop — continues across segments
  t[static_cast<unsigned>('\\')] = k_slash_only;
  t[static_cast<unsigned>('?')] = k_termination;
  t[static_cast<unsigned>('#')] = k_termination;
  t[static_cast<unsigned>(' ')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('"')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('<')] = k_encoding | k_stop_token;
  t[static_cast<unsigned>('>')] = k_encoding | k_stop_token;
  return t;
}

constexpr auto segment_table = make_segment_table();
constexpr auto region_table = make_region_table();
constexpr std::uint8_t stop_token = k_stop_token;

// ---------------------------------------------------------------------------
// Scalar / unrolled implementations
// ---------------------------------------------------------------------------

template <typename T, std::size_t N, typename Iter>
[[nodiscard]] __attribute__((noinline)) T
or_all_scalar(std::array<T, N> const &arr, T const stop, Iter &pos,
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
      break;
    }
  }
  return res;
}

template <typename T, std::size_t N, typename Iter>
[[nodiscard]] __attribute__((noinline)) T
or_all_unrolled(std::array<T, N> const &arr, T const stop, Iter &pos,
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
      return res;
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
// Fixtures that actually scan
// ---------------------------------------------------------------------------

struct Fixture {
  std::string data;
  char const *name;
  std::array<std::uint8_t, 256> const *table; // segment vs region
};

std::string make_run(char fill, std::size_t n, char stop = '\0') {
  std::string s(n, fill);
  if (stop != '\0') {
    s.push_back(stop);
    s.append("TAIL");
  }
  return s;
}

std::string make_plain_path_no_leading_slash(std::size_t segs,
                                             std::size_t seg_len) {
  // "aaaa/bbbb/cccc/..." — first char is NOT a stop in the segment table
  std::string s;
  s.reserve(segs * (seg_len + 1));
  for (std::size_t i = 0; i < segs; ++i) {
    if (i) {
      s.push_back('/');
    }
    for (std::size_t j = 0; j < seg_len; ++j) {
      s.push_back(static_cast<char>('a' + static_cast<int>((i + j) % 26)));
    }
  }
  return s;
}

std::vector<Fixture> const &fixtures() {
  static std::vector<Fixture> f = {
      // --- region table: '/' does not stop; long bulk scan ---
      {make_plain_path_no_leading_slash(32, 16), "region_long_path",
       &region_table}, // ~543 B
      {make_plain_path_no_leading_slash(8, 8), "region_medium_path",
       &region_table},
      {make_run('a', 512, '?'), "region_512_then_query", &region_table},
      {make_run('a', 64, '#'), "region_64_then_frag", &region_table},
      {make_run('a', 512), "region_512_no_stop", &region_table},

      // --- segment table: stops on first '/' (one segment) ---
      {make_run('a', 64, '/'), "segment_64_then_slash", &segment_table},
      {make_run('a', 16, '/'), "segment_16_then_slash", &segment_table},
      {make_run('a', 4, '?'), "segment_4_then_query", &segment_table},
      {std::string(128, 'x') + "://host.example/path", "host_like",
       &segment_table},
  };
  return f;
}

void assert_equivalent() {
  for (auto const &fx : fixtures()) {
    auto const *beg = fx.data.data();
    auto const *end = beg + fx.data.size();
    auto const &tab = *fx.table;

    auto *p0 = beg;
    auto r0 = or_all_scalar(tab, stop_token, p0, end);
    auto *p1 = beg;
    auto r1 = or_all_unrolled(tab, stop_token, p1, end);

    if (r0 != r1 || p0 != p1) {
      std::fprintf(stderr, "mismatch '%s': scalar(%u,%td) unrolled(%u,%td)\n",
                   fx.name, unsigned(r0), p0 - beg, unsigned(r1), p1 - beg);
      std::abort();
    }
  }
}

struct Once {
  Once() { assert_equivalent(); }
};
Once const g_once{};

// ---------------------------------------------------------------------------
// Benchmarks — noinline + volatile sink to defeat LTO/DCE
// ---------------------------------------------------------------------------

void BM_OrAll_Scalar(benchmark::State &state) {
  auto const &fx = fixtures()[static_cast<std::size_t>(state.range(0))];
  auto const *beg = fx.data.data();
  auto const *end = beg + fx.data.size();
  auto const &tab = *fx.table;
  std::uint64_t sink = 0;

  for (auto _ : state) {
    auto *pos = beg;
    auto st = or_all_scalar(tab, stop_token, pos, end);
    sink += static_cast<std::uint64_t>(st);
    sink += static_cast<std::uint64_t>(pos - beg);
    benchmark::DoNotOptimize(sink);
    benchmark::DoNotOptimize(pos);
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
  auto const &tab = *fx.table;
  std::uint64_t sink = 0;

  for (auto _ : state) {
    auto *pos = beg;
    auto st = or_all_unrolled(tab, stop_token, pos, end);
    sink += static_cast<std::uint64_t>(st);
    sink += static_cast<std::uint64_t>(pos - beg);
    benchmark::DoNotOptimize(sink);
    benchmark::DoNotOptimize(pos);
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
