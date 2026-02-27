#include <benchmark/benchmark.h>

using namespace std;

#include <benchmark/benchmark.h>

#include <cstdint>
#include <array>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────

namespace stl {
    using namespace std;
}

// Your original functions ────────────────────────────────────────────────────

template <typename IntegerType = int, bool SupportUppercase = true, bool SupportHex = true>
[[nodiscard]] static constexpr IntegerType hex_digit_safe(
    char const  inp_char,
    IntegerType default_value = static_cast<IntegerType>(-1)) noexcept 
{
    static constexpr auto hex_table = []() consteval -> stl::array<stl::int8_t, 256> {
        stl::array<stl::int8_t, 256> table{};
        table.fill(-1);

        for (stl::uint8_t i = '0'; i <= '9'; ++i) {
            table.at(i) = static_cast<stl::int8_t>(i - '0');
        }

        if constexpr (SupportHex) {
            for (stl::uint8_t i = 'a'; i <= 'f'; ++i) {
                table.at(i) = static_cast<stl::int8_t>(10 + (i - 'a'));
            }
            if constexpr (SupportUppercase) {
                for (stl::uint8_t i = 'A'; i <= 'F'; ++i) {
                    table.at(i) = static_cast<stl::int8_t>(10 + (i - 'A'));
                }
            }
        }
        return table;
    }();

    auto const val = hex_table[static_cast<stl::uint8_t>(inp_char)];
    return val >= 0 ? static_cast<IntegerType>(val) : default_value;
}


template <typename IntegerType = int, bool SupportUppercase = true, bool SupportHex = true, typename CharT = char>
[[nodiscard]] static constexpr IntegerType hex_digit_safe2(
    CharT       inp_char,
    IntegerType default_value = static_cast<IntegerType>(-1)) noexcept 
{
    if (static_cast<CharT>('0') <= inp_char && inp_char <= static_cast<CharT>('9')) {
        return static_cast<IntegerType>(inp_char - static_cast<CharT>('0'));
    }
    if constexpr (SupportHex) {
        if (static_cast<CharT>('a') <= inp_char && inp_char <= static_cast<CharT>('f')) {
            return static_cast<IntegerType>(inp_char - static_cast<CharT>('a') + static_cast<CharT>(10));
        }
        if constexpr (SupportUppercase) {
            if (static_cast<CharT>('A') <= inp_char && inp_char <= static_cast<CharT>('F')) {
                return static_cast<IntegerType>(inp_char - static_cast<CharT>('A') + static_cast<CharT>(10));
            }
        }
    }
    return default_value;
}

// ─────────────────────────────────────────────────────────────────────────────
// Benchmark drivers
// ─────────────────────────────────────────────────────────────────────────────

static void BM_Lookup_Table(benchmark::State& state) {
    const char* data = "0123456789abcdefABCDEF!@#$%^&*()_+ghijklmno";
    const std::size_t len = std::strlen(data);
    int32_t sum = 0;

    for (auto _ : state) {
        for (std::size_t i = 0; i < len; ++i) {
            sum += hex_digit_safe<int, true, true>(data[i]);
            benchmark::DoNotOptimize(sum);
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * len);
    state.SetBytesProcessed(state.iterations() * len);
    benchmark::ClobberMemory();
}
// ---------------------------------------------------------------------------

static void BM_If_Ladder(benchmark::State& state) {
    const char* data = "0123456789abcdefABCDEF!@#$%^&*()_+ghijklmno";
    const std::size_t len = std::strlen(data);
    int32_t sum = 0;

    for (auto _ : state) {
        for (std::size_t i = 0; i < len; ++i) {
            sum += hex_digit_safe2<int, true, true>(data[i]);
            benchmark::DoNotOptimize(sum);
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * len);
    state.SetBytesProcessed(state.iterations() * len);
    benchmark::ClobberMemory();
}
// ---------------------------------------------------------------------------

static void BM_Lookup_Table_OnlyLower(benchmark::State& state) {
    const char* data = "0123456789abcdef.....ghijklmnopqrs.....";
    const std::size_t len = std::strlen(data);
    int32_t sum = 0;

    for (auto _ : state) {
        for (std::size_t i = 0; i < len; ++i) {
            sum += hex_digit_safe<int, false, true>(data[i]);
            benchmark::DoNotOptimize(sum);
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * len);
    state.SetBytesProcessed(state.iterations() * len);
}
// ---------------------------------------------------------------------------

static void BM_If_Ladder_OnlyLower(benchmark::State& state) {
    const char* data = "0123456789abcdef.....ghijklmnopqrs.....";
    const std::size_t len = std::strlen(data);
    int32_t sum = 0;

    for (auto _ : state) {
        for (std::size_t i = 0; i < len; ++i) {
            sum += hex_digit_safe2<int, false, true>(data[i]);
            benchmark::DoNotOptimize(sum);
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * len);
    state.SetBytesProcessed(state.iterations() * len);
}

// ─────────────────────────────────────────────────────────────────────────────

BENCHMARK(BM_Lookup_Table)->Name("lookup (0-9a-fA-F)");
BENCHMARK(BM_If_Ladder)   ->Name("if-ladder (0-9a-fA-F)");

BENCHMARK(BM_Lookup_Table_OnlyLower)->Name("lookup (0-9a-f only)");
BENCHMARK(BM_If_Ladder_OnlyLower)   ->Name("if-ladder (0-9a-f only)");

// Try to reduce branch predictor advantage of one version
BENCHMARK(BM_Lookup_Table)->Name("lookup (mixed)")->Arg(1);
BENCHMARK(BM_If_Ladder)   ->Name("if-ladder (mixed)")->Arg(1);

// ─────────────────────────────────────────────────────────────────────────────

// BENCHMARK_MAIN();
