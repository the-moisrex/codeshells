#include <benchmark/benchmark.h>
#include <cstdint>
#include <cstring>
#include <array>
#include <random>
#include <vector>

// Enum mimicking the Swift proposal
enum class CheckResult {
  LowercasedASCII,    // Definitely ASCII and all lowercase (or non-letter)
  AsciiPossiblyUpper, // ASCII but might contain uppercase letters
  NotASCII            // Contains bytes > 127
};

// The proposed idea: scalar loop with bitwise OR and AND
CheckResult bitwise_check(const uint8_t *data, size_t len) {
  if (len == 0)
    return CheckResult::LowercasedASCII;

  uint8_t or_result = 0;
  uint8_t and_result = 0xFF;

  for (size_t i = 0; i < len; ++i) {
    uint8_t byte = data[i];
    or_result |= byte;
    and_result &= byte;
  }

  bool is_ascii = (or_result <= 127);

  // Bit 5 (0b00100000) is set in a-z and most lowercase-allowed chars,
  // cleared in A-Z. If AND has bit 5 set, no uppercase letter present.
  bool definitely_lowercased = (and_result & 0b00100000) != 0;

  if (is_ascii && definitely_lowercased) {
    return CheckResult::LowercasedASCII;
  } else if (is_ascii) {
    return CheckResult::AsciiPossiblyUpper;
  } else {
    return CheckResult::NotASCII;
  }
}

// Traditional alternative: full iteration checking for uppercase A-Z
bool has_uppercase_ascii(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    uint8_t b = data[i];
    if (b >= 'A' && b <= 'Z') {
      return true;
    }
  }
  return false;
}

CheckResult traditional_check(const uint8_t *data, size_t len) {
  uint8_t max_byte = 0;
  for (size_t i = 0; i < len; ++i) {
    uint8_t b = data[i];
    if (b > max_byte)
      max_byte = b;
  }

  if (max_byte > 127)
    return CheckResult::NotASCII;

  // ASCII confirmed, now check for any uppercase
  if (has_uppercase_ascii(data, len)) {
    return CheckResult::AsciiPossiblyUpper;
  } else {
    return CheckResult::LowercasedASCII;
  }
}

// Table-lookup variant for detecting uppercase (fast branch-free per byte)
bool has_uppercase_table(const uint8_t *data, size_t len) {
  static const auto is_upper = []() consteval {
    std::array<bool, 256> table{};
    for (int i = 'A'; i <= 'Z'; ++i)
      table[i] = true;
    return table;
  }();

  for (size_t i = 0; i < len; ++i) {
    if (is_upper[data[i]])
      return true;
  }
  return false;
}

CheckResult table_check(const uint8_t *data, size_t len) {
  uint8_t max_byte = 0;
  for (size_t i = 0; i < len; ++i) {
    uint8_t b = data[i];
    if (b > max_byte)
      max_byte = b;
  }

  if (max_byte > 127)
    return CheckResult::NotASCII;

  if (has_uppercase_table(data, len)) {
    return CheckResult::AsciiPossiblyUpper;
  } else {
    return CheckResult::LowercasedASCII;
  }
}

// Generate test data with different characteristics
std::vector<uint8_t> generate_data(size_t len, double prob_non_ascii,
                                   double prob_upper_given_ascii) {
  std::mt19937 rng(12345);
  std::uniform_int_distribution<int> dist_ascii(0, 127);
  std::uniform_int_distribution<int> dist_full(0, 255);

  std::vector<uint8_t> data(len);
  for (size_t i = 0; i < len; ++i) {
    if (static_cast<double>(rng()) / rng.max() < prob_non_ascii) {
      data[i] = dist_full(rng);
    } else {
      uint8_t b = dist_ascii(rng);
      // Roughly bias toward lowercase/non-letters
      if (b >= 'A' && b <= 'Z' &&
          static_cast<double>(rng()) / rng.max() >
              (1.0 - prob_upper_given_ascii)) {
        // keep uppercase
      } else if (b >= 'a' && b <= 'z') {
        b -= 32; // make uppercase? No: to control prob_upper
                 // Better: decide first
                 // Simpler: pick uniform 0-127 then flip to upper with prob
      }
      // Correct way:
      uint8_t base = dist_ascii(rng);
      if (base >= 'a' && base <= 'z' &&
          static_cast<double>(rng()) / rng.max() < prob_upper_given_ascii) {
        base -= 32; // make uppercase
      } else if (base >= 'A' && base <= 'Z' &&
                 static_cast<double>(rng()) / rng.max() >=
                     prob_upper_given_ascii) {
        base += 32; // make lowercase
      }
      data[i] = base;
    }
  }
  return data;
}

// Benchmarks
static void BM_Bitwise(benchmark::State &state) {
  auto data =
      generate_data(state.range(0), 0.01, 0.1); // mostly ASCII, few uppercase
  for (auto _ : state) {
    benchmark::DoNotOptimize(bitwise_check(data.data(), data.size()));
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}

static void BM_Traditional(benchmark::State &state) {
  auto data = generate_data(state.range(0), 0.01, 0.1);
  for (auto _ : state) {
    benchmark::DoNotOptimize(traditional_check(data.data(), data.size()));
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}

static void BM_Table(benchmark::State &state) {
  auto data = generate_data(state.range(0), 0.01, 0.1);
  for (auto _ : state) {
    benchmark::DoNotOptimize(table_check(data.data(), data.size()));
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}

// Register benchmarks with range of input sizes
BENCHMARK(BM_Bitwise)->Range(8, 1 << 16);
BENCHMARK(BM_Traditional)->Range(8, 1 << 16);
BENCHMARK(BM_Table)->Range(8, 1 << 16);
