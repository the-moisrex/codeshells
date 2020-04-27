#include <atomic>
#include <benchmark/benchmark.h>
#include "task.hpp"

using namespace std;

static void Task(benchmark::State& state) {
  for (auto _ : state) {
    task<int()> _task = [] {
        return 20;
    };
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Task);



static void Function(benchmark::State& state) {
  for (auto _ : state) {
    std::function<int()> _task = [] {
        return 20;
    };
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Function);



static void Function2(benchmark::State& state) {
  std::function<int()> _task = [] {
      return 20;
  };
  for (auto _ : state) {
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Function2);




static void Task2(benchmark::State& state) {
  task<int()> _task = [] {
      return 20;
  };
  for (auto _ : state) {
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Task2);




static void Function3(benchmark::State& state) {
  std::function<std::string()> _task = [] {
      return std::to_string(20);
  };
  for (auto _ : state) {
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Function3);




static void Task3(benchmark::State& state) {
  task<std::string()> _task = [] {
      return std::to_string(20);
  };
  for (auto _ : state) {
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Task3);





static void Function4Mutable(benchmark::State& state) {
  std::function<int()> _task = [a = 0] () mutable {
      return a++;
  };
  for (auto _ : state) {
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Function4Mutable);




static void Task4Mutable(benchmark::State& state) {
  task<int()> _task = [a=0] () mutable {
      return a++;
  };
  for (auto _ : state) {
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Task4Mutable);












static void Function5Mutable(benchmark::State& state) {
  for (auto _ : state) {
    std::function<int()> _task = [a = 0] () mutable {
        return a++;
    };
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Function5Mutable);




static void Task5Mutable(benchmark::State& state) {
  for (auto _ : state) {
    task<int()> _task = [a=0] () mutable {
        return a++;
    };
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(Task5Mutable);




static void lambdaMutable(benchmark::State& state) {
  for (auto _ : state) {
    auto _task = [a = 0] () mutable {
        return a++;
    };
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(lambdaMutable);




static void lambdaStatic(benchmark::State& state) {
  auto _task = [] {
      return 20;
  };
  for (auto _ : state) {
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(lambdaStatic);

static void lambda(benchmark::State& state) {
  auto _task = [a=0] () mutable {
      return a++;
  };
  for (auto _ : state) {
    benchmark::DoNotOptimize(_task);
    benchmark::DoNotOptimize(_task());
  }
}
BENCHMARK(lambda);
