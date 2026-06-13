#include <benchmark/benchmark.h>

using namespace std;

#include <algorithm>
#include <array>
#include <benchmark/benchmark.h>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <string>
#include <type_traits>

namespace stl {
using namespace std;
}

// ============================================================================
// Your Provided Code
// ============================================================================

template <stl::integral T = stl::uint32_t, stl::size_t N,
          stl::random_access_iterator Iter>
[[nodiscard]] static constexpr T or_all(stl::array<T, N> const &arr, Iter &pos,
                                        Iter end, T stop_token) noexcept {
  static_assert(N <= 256,
                "We cast to uint8_t, which means you can't do more than 255");
  using char_type = stl::make_unsigned_t<stl::iter_value_t<Iter>>;
  constexpr auto last_el = static_cast<char_type>(N - 1U);
  T res{};

  while (end - pos >= 4) {
    T v0 = arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(pos[0]), last_el))];
    T v1 = arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(pos[1]), last_el))];
    T v2 = arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(pos[2]), last_el))];
    T v3 = arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(pos[3]), last_el))];

    T batch_res = v0 | v1 | v2 | v3;

    if ((batch_res & stop_token) != T{0}) {
      if ((v0 & stop_token) != T{0}) {
        res |= v0;
        return res;
      }
      if ((v1 & stop_token) != T{0}) {
        res |= v0 | v1;
        pos += 1;
        return res;
      }
      if ((v2 & stop_token) != T{0}) {
        res |= v0 | v1 | v2;
        pos += 2;
        return res;
      }
      res |= batch_res;
      pos += 3;
      return res;
    }

    res |= batch_res;
    pos += 4;
  }

  while (pos != end) {
    T v = arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(*pos), last_el))];
    res |= v;
    if ((v & stop_token) != T{0}) {
      break;
    }
    ++pos;
  }
  return res;
}

template <stl::integral T = stl::uint32_t, stl::size_t N,
          stl::random_access_iterator Iter>
[[nodiscard]] static constexpr T or_all(stl::array<T, N> const &arr, Iter pos,
                                        Iter end) noexcept {
  static_assert(N <= 256,
                "We cast to uint8_t, which means you can't do more than 255");
  using char_type = stl::make_unsigned_t<stl::iter_value_t<Iter>>;
  constexpr auto last_el = static_cast<char_type>(N - 1U);
  T res{};
  while (stl::next(pos, 4) <= end) {
    res |= arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(*pos++), last_el))];
    res |= arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(*pos++), last_el))];
    res |= arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(*pos++), last_el))];
    res |= arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(*pos++), last_el))];
  }
  for (; pos != end; ++pos) {
    res |= arr[static_cast<stl::uint8_t>(
        stl::min<char_type>(static_cast<char_type>(*pos), last_el))];
  }
  return res;
}

template <stl::integral T = stl::uint32_t, stl::size_t N,
          stl::random_access_iterator Iter>
[[nodiscard]] static constexpr T or_all2(stl::array<T, N> const &arr, Iter &pos,
                                         Iter end, T stop_token) noexcept {
  static_assert(N <= 256,
                "We cast to uint8_t, which means you can't do more than 255");
  using char_type = stl::make_unsigned_t<stl::iter_value_t<Iter>>;
  constexpr auto last_el = static_cast<char_type>(N - 1U);
  T res{};
  constexpr int VEC_SIZE = 8;

  while (end - pos >= VEC_SIZE) {
    T vals[VEC_SIZE];
    T batch_res{};

#pragma clang loop vectorize(enable) interleave(enable)
#pragma GCC ivdep
    for (int i = 0; i < VEC_SIZE; ++i) {
      char_type c = static_cast<char_type>(pos[i]);
      char_type idx = (c < last_el) ? c : last_el;
      vals[i] = arr[static_cast<stl::uint8_t>(idx)];
      batch_res |= vals[i];
    }

    if ((batch_res & stop_token) != T{0}) {
      for (int i = 0; i < VEC_SIZE; ++i) {
        res |= vals[i];
        if ((vals[i] & stop_token) != T{0}) {
          pos += i;
          return res;
        }
      }
    }

    res |= batch_res;
    pos += VEC_SIZE;
  }

  while (pos != end) {
    char_type c = static_cast<char_type>(*pos);
    char_type idx = (c < last_el) ? c : last_el;
    T v = arr[static_cast<stl::uint8_t>(idx)];
    res |= v;
    if ((v & stop_token) != T{0}) {
      break;
    }
    ++pos;
  }
  return res;
}

// ============================================================================
// Benchmarks
// ============================================================================

constexpr uint32_t STOP_TOKEN_VAL = 0x80000000;

// Helper to generate the mapping
std::array<uint32_t, 256> generate_mapping() {
  std::array<uint32_t, 256> mapping{};
  for (int i = 0; i < 256; ++i) {
    mapping[i] = (i % 2 == 0) ? 0x1 : 0x2; // Dummy safe values
  }
  mapping[static_cast<uint8_t>(';')] =
      STOP_TOKEN_VAL; // ';' acts as our stop token
  return mapping;
}

// 1. Benchmark: Stop token is at the very end of the buffer
static void BM_LateStop_or_all1(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, 'a');
  data.back() = ';'; // Stop token at the end
  auto mapping = generate_mapping();

  for (auto _ : state) {
    auto pos = data.data();
    auto end = pos + size;
    auto res = or_all(mapping, pos, end, STOP_TOKEN_VAL);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(pos);
  }
  state.SetBytesProcessed(int64_t(state.iterations()) * size);
}

static void BM_LateStop_or_all2(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, 'a');
  data.back() = ';';
  auto mapping = generate_mapping();

  for (auto _ : state) {
    auto pos = data.data();
    auto end = pos + size;
    auto res = or_all2(mapping, pos, end, STOP_TOKEN_VAL);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(pos);
  }
  state.SetBytesProcessed(int64_t(state.iterations()) * size);
}

// 2. Benchmark: Stop token is early in the buffer (index 4)
static void BM_EarlyStop_or_all1(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, 'a');
  if (size > 4)
    data[4] = ';'; // Stop early
  auto mapping = generate_mapping();

  for (auto _ : state) {
    auto pos = data.data();
    auto end = pos + size;
    auto res = or_all(mapping, pos, end, STOP_TOKEN_VAL);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(pos);
  }
}

static void BM_EarlyStop_or_all2(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, 'a');
  if (size > 4)
    data[4] = ';';
  auto mapping = generate_mapping();

  for (auto _ : state) {
    auto pos = data.data();
    auto end = pos + size;
    auto res = or_all2(mapping, pos, end, STOP_TOKEN_VAL);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(pos);
  }
}

// 3. Benchmark: Without Stop Token (processes full array)
static void BM_NoStopToken_or_all(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, 'a');
  auto mapping = generate_mapping();

  for (auto _ : state) {
    auto pos = data.data();
    auto end = pos + size;
    auto res = or_all(mapping, pos, end);
    benchmark::DoNotOptimize(res);
  }
  state.SetBytesProcessed(int64_t(state.iterations()) * size);
}

#include <immintrin.h>

// ============================================================================
// New SIMD Implementation
// ============================================================================

char res_new_simd(char *&pos, const char *end, char stop_token) {
  char global_res = 0;

  // Load the lookup table into a 16-byte SIMD register, padded with zeros
  const __m128i table = _mm_setr_epi8('1', '2', '3', 'd', '6', '5', 'h', 'b',
                                      's', 0, 0, 0, 0, 0, 0, 0);
  const __m128i limit = _mm_set1_epi8(9);
  const __m128i stop_vec = _mm_set1_epi8(stop_token);

  // Process $16$ bytes per iteration
  while (end - pos >= 16) {
    __m128i data = _mm_loadu_si128((const __m128i *)pos);

    // Cap index at 9: min(data, 9)
    __m128i idx = _mm_min_epu8(data, limit);

    // In-register table lookup: maps each byte in 'idx' to 'table[idx]'
    __m128i mapped = _mm_shuffle_epi8(table, idx);

    // Check if stop_token exists anywhere in these $16$ bytes
    __m128i match = _mm_cmpeq_epi8(mapped, stop_vec);
    int mask = _mm_movemask_epi8(match);

    if (mask != 0) {
      // Stop token found: fallback to sequential for the exact stopping point
      int offset =
          __builtin_ctz(mask); // Count trailing zeros to find exact index
      for (int i = 0; i <= offset; ++i) {
        global_res |= ((const char *)&mapped)[i];
      }
      pos += offset;
      return global_res;
    }

    // Accumulate mapped values
    uint64_t low = _mm_extract_epi64(mapped, 0);
    uint64_t high = _mm_extract_epi64(mapped, 1);

    // Fold bytes into a single OR
    low |= high;
    low |= (low >> 32);
    low |= (low >> 16);
    low |= (low >> 8);
    global_res |= (char)low;

    pos += 16;
  }

  // Sequential fallback for the remaining < $16$ bytes
  const char *interesting = "123d65hbs";
  while (pos < end) {
    char val = *pos;
    int idx = (unsigned char)val > 9 ? 9 : (unsigned char)val;
    char mapped = interesting[idx];

    global_res |= mapped;
    if ((mapped & stop_token) == stop_token) {
      break;
    }
    pos++;
  }

  return global_res;
}

/**
 * Usage:
 *   auto mapping = categorize(...);
 *   switch (or_all_autovec(mapping, pos, end, STOP_TOKEN)) {
 *     case ...: ...;
 *     case ...: ...;
 *   }
 */
template <stl::integral T = stl::uint32_t, stl::size_t N,
          stl::random_access_iterator Iter>
[[nodiscard]] static constexpr T or_all_autovec(stl::array<T, N> const &arr,
                                                Iter &pos, Iter end,
                                                T stop_token) noexcept {
  static_assert(N <= 256,
                "We cast to uint8_t, which means you can't do more than 255");
  using char_type = stl::make_unsigned_t<stl::iter_value_t<Iter>>;
  constexpr auto last_el = static_cast<char_type>(N - 1U);
  T res{};

  // 16 or 32 is optimal for 128-bit / 256-bit SIMD registers respectively.
  constexpr stl::ptrdiff_t VEC_SIZE = 16;

  while (end - pos >= VEC_SIZE) {
    T vals[VEC_SIZE];
    T batch_res{};
    T stop_mask{};

// 1. PURE VECTORIZABLE INNER LOOP
// No branches, no early exits. Computes mapping, accumulation, and stop
// trigger.
#pragma clang loop vectorize(enable) interleave(enable)
#pragma GCC ivdep
    for (stl::ptrdiff_t i = 0; i < VEC_SIZE; ++i) {
      char_type c = static_cast<char_type>(pos[i]);
      // Branchless min limits the index safely for table lookup
      char_type idx = (c < last_el) ? c : last_el;

      vals[i] = arr[static_cast<stl::uint8_t>(idx)];
      batch_res |= vals[i];
      stop_mask |= (vals[i] & stop_token);
    }

    // 2. CHECK IF STOP TOKEN WAS TRIGGERED ANYWHERE IN BATCH
    if (stop_mask != T{0}) {
      // 3. FALLBACK TO FIND EXACT STOPPING POINT
      for (stl::ptrdiff_t i = 0; i < VEC_SIZE; ++i) {
        res |= vals[i];
        if ((vals[i] & stop_token) != T{0}) {
          pos += i; // Update pos to the exact trigger element
          return res;
        }
      }
    }

    res |= batch_res;
    pos += VEC_SIZE;
  }

  // Tail processing for remaining elements (less than VEC_SIZE)
  while (pos != end) {
    char_type c = static_cast<char_type>(*pos);
    char_type idx = (c < last_el) ? c : last_el;
    T v = arr[static_cast<stl::uint8_t>(idx)];
    res |= v;
    if ((v & stop_token) != T{0}) {
      break;
    }
    ++pos;
  }

  return res;
}

// ============================================================================
// New Benchmarks
// ============================================================================

// SIMD uses 0-9 as input indices. 0 maps to '1', 8 maps to 's'.
constexpr char SIMD_STOP_TOKEN = 's';
constexpr char SIMD_STOP_INDEX = 8;
constexpr char SIMD_SAFE_INDEX = 0;

static void BM_LateStop_simd(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, SIMD_SAFE_INDEX);
  data.back() = SIMD_STOP_INDEX;

  for (auto _ : state) {
    char *pos = data.data();
    const char *end = pos + size;
    auto res = res_new_simd(pos, end, SIMD_STOP_TOKEN);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(pos);
  }
  state.SetBytesProcessed(int64_t(state.iterations()) * size);
}

static void BM_EarlyStop_simd(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, SIMD_SAFE_INDEX);
  if (size > 4)
    data[4] = SIMD_STOP_INDEX;

  for (auto _ : state) {
    char *pos = data.data();
    const char *end = pos + size;
    auto res = res_new_simd(pos, end, SIMD_STOP_TOKEN);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(pos);
  }
}

static void BM_EarlyStop_or_all4(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, 'a');
  if (size > 4)
    data[4] = ';';
  auto mapping = generate_mapping();

  for (auto _ : state) {
    auto pos = data.data();
    auto end = pos + size;
    auto res = or_all_autovec(mapping, pos, end, STOP_TOKEN_VAL);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(pos);
  }
}

static void BM_LateStop_or_all4(benchmark::State &state) {
  const size_t size = state.range(0);
  std::string data(size, 'a');
  data.back() = ';';
  auto mapping = generate_mapping();

  for (auto _ : state) {
    auto pos = data.data();
    auto end = pos + size;
    auto res = or_all_autovec(mapping, pos, end, STOP_TOKEN_VAL);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(pos);
  }
  state.SetBytesProcessed(int64_t(state.iterations()) * size);
}

// Register new benchmarks
BENCHMARK(BM_LateStop_simd)->Range(64, 8 << 10);
BENCHMARK(BM_EarlyStop_simd)->Range(64, 8 << 10);

BENCHMARK(BM_LateStop_or_all4)->Range(64, 8 << 10);
BENCHMARK(BM_EarlyStop_or_all4)->Range(64, 8 << 10);

// Register benchmarks with sizes from 64 bytes to 8 kilobytes
BENCHMARK(BM_LateStop_or_all1)->Range(64, 8 << 10);
BENCHMARK(BM_LateStop_or_all2)->Range(64, 8 << 10);

BENCHMARK(BM_EarlyStop_or_all1)->Range(64, 8 << 10);
BENCHMARK(BM_EarlyStop_or_all2)->Range(64, 8 << 10);

BENCHMARK(BM_NoStopToken_or_all)->Range(64, 8 << 10);

// BENCHMARK_MAIN();
