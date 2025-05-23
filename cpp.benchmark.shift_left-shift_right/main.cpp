#include <benchmark/benchmark.h>

#include <algorithm>
#include <benchmark/benchmark.h>
#include <vector>
#include <random>

// Helper to generate a vector with random integers
std::vector<int> generate_data(size_t size) {
    std::vector<int> data(size);
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, 100);
    for (auto& x : data) {
        x = dis(gen);
    }
    return data;
}

static void BM_shift_left(benchmark::State& state) {
    size_t size = state.range(0);
    std::vector<int> data = generate_data(size);
    size_t shift_amount = 10;

    for (auto _ : state) {
        // Copy original data to avoid modifying it between iterations
        std::vector<int> v = data;
        benchmark::DoNotOptimize(v.data());
        std::shift_left(v.begin(), v.end(), shift_amount);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * size);
}

static void BM_shift_right(benchmark::State& state) {
    size_t size = state.range(0);
    std::vector<int> data = generate_data(size);
    size_t shift_amount = 10;

    for (auto _ : state) {
        std::vector<int> v = data;
        benchmark::DoNotOptimize(v.data());
        std::shift_right(v.begin(), v.end(), shift_amount);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * size);
}

BENCHMARK(BM_shift_left)->RangeMultiplier(10)->Range(1000, 100000);
BENCHMARK(BM_shift_right)->RangeMultiplier(10)->Range(1000, 100000);



static void BM2_shift_left(benchmark::State& state) {
    std::vector<int> data = generate_data(1000);
    size_t shift_amount = state.range(0);

    for (auto _ : state) {
        // Copy original data to avoid modifying it between iterations
        std::vector<int> v = data;
        benchmark::DoNotOptimize(v.data());
        std::shift_left(v.begin(), v.end(), shift_amount);
        benchmark::ClobberMemory();
    }
}

static void BM2_shift_right(benchmark::State& state) {
    std::vector<int> data = generate_data(1000);
    size_t shift_amount = state.range(0);

    for (auto _ : state) {
        std::vector<int> v = data;
        benchmark::DoNotOptimize(v.data());
        std::shift_right(v.begin(), v.end(), shift_amount);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM2_shift_left)->RangeMultiplier(2)->Range(10, 900);
BENCHMARK(BM2_shift_right)->RangeMultiplier(2)->Range(10, 900);



static void BM3_shift_left(benchmark::State& state) {
    std::vector<int> data = generate_data(1000);
    size_t shift_amount = 100;
    size_t count = state.range(0);

    for (auto _ : state) {
        // Copy original data to avoid modifying it between iterations
        std::vector<int> v = data;
        benchmark::DoNotOptimize(v.data());
        std::shift_left(v.begin(), v.begin() + count, shift_amount);
        benchmark::ClobberMemory();
    }
}

static void BM3_shift_right(benchmark::State& state) {
    std::vector<int> data = generate_data(1000);
    size_t shift_amount = 100;
    size_t count = state.range(0);

    for (auto _ : state) {
        std::vector<int> v = data;
        benchmark::DoNotOptimize(v.data());
        std::shift_right(v.begin(), v.begin() + count, shift_amount);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM3_shift_left)->RangeMultiplier(2)->Range(10, 900);
BENCHMARK(BM3_shift_right)->RangeMultiplier(2)->Range(10, 900);

