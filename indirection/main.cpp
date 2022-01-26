#include <benchmark/benchmark.h>
#include <functional>
#include <vector>

using namespace std;

static void EmptyRandom(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(1);
  }
}
BENCHMARK(EmptyRandom);


int n = 1;
int direct_func2(int a) noexcept {
    return (a++) + (n++);
};
int direct_func(int a) noexcept {
    return direct_func2(a);
};

static void DirectCall(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(direct_func(1));
  }
}
BENCHMARK(DirectCall);


static void InDirectCall(benchmark::State& state) {
  function<int(int)> func2 = [n=1] (int a) mutable {
    return (a++) + (n++);
  };
  function<int(int)> func = [=] (int a) {
    return func2(a);
  };

  for (auto _ : state) {
    benchmark::DoNotOptimize(func(1));
  }
}
BENCHMARK(InDirectCall);





/////////////////////////////

struct a_router_t {
  vector<function<int(int)>> funcs;
};

struct a_application {
  a_router_t router;
};



static void InDirectWebppCall(benchmark::State& state) {
  function<int(int)> func2 = [n=1] (int a) mutable {
    return (a++) + (n++);
  };

  a_application app;
  app.router.funcs.push_back([=] (int a) {return func2(a);});

  for (auto _ : state) {
    benchmark::DoNotOptimize(app.router.funcs[0](1));
  }
}
BENCHMARK(InDirectWebppCall);



////////////////////////////


template <typename Func>
struct b_router_t {
  Func func;
};

template <typename Function>
struct b_application {
  b_router_t<Function> router;
};



static void DirectWebppCall(benchmark::State& state) {
  function<int(int)> func2 = [n=1] (int a) mutable {
    return (a++) + (n++);
  };

  auto func = [=] (int a) {
    return func2(a);
  };

  b_application<decltype(func)> app{func};

  for (auto _ : state) {
    benchmark::DoNotOptimize(app.router.func(1));
  }
}
BENCHMARK(DirectWebppCall);





