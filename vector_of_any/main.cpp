#include <any>
#include <benchmark/benchmark.h>
#include <optional>
#include <vector>

using namespace std;

template <typename T> std::optional<T> find(const std::vector<std::any> &v) {
  for (auto &&e : v) {
    if (auto ptr = std::any_cast<T>(&e)) {
      return *ptr;
    }
  }

  return std::nullopt;
}


template <typename T>
T const* findPtr(const std::vector<std::any> &v) {
  for (auto &&e : v) {
    if (auto ptr = std::any_cast<T>(&e)) {
      return ptr;
    }
  }
  return nullptr;
}


template <typename T> std::vector<T> findAll(const std::vector<std::any> &v) {
  std::vector<T> out;
  for (auto &&e : v) {
    if (auto ptr = std::any_cast<T>(&e)) {
      out.push_back(*ptr);
    }
  }

  return out;
}

static void EmptyRandom(benchmark::State &state) {
  for (auto _ : state) {
    // benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(EmptyRandom);

static void anyCast(benchmark::State &state) {
  any a = float{rand() * 6.4}; // NOLINT
  for (auto _ : state) {
    float c;
    if (auto *res = any_cast<float>(&a); res != nullptr)
      c = *res;
    else
      c = 0.0f;
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(anyCast);

static void vecOfAny(benchmark::State &state) {
  vector<any> vec;
  vec.emplace_back(int{10});
  vec.emplace_back(float{10.1});
  for (auto _ : state) {
    float c = find<float>(vec).value_or(0.0f);
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(vecOfAny);


static void vecOfAnyPtr(benchmark::State &state) {
  vector<any> vec;
  vec.emplace_back(int{10});
  vec.emplace_back(float{10.1});
  for (auto _ : state) {
    float c;
    if (auto* res = findPtr<float>(vec); res != nullptr)
        c = *res;
    else
        c = 0.0f;
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(vecOfAnyPtr);
