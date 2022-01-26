#include <benchmark/benchmark.h>

#include <array>
#include <variant>
#include <random>
#include <functional>
#include <algorithm>

using namespace std;

struct One {
  auto get () const { return 1; }
 };
struct Two {
  auto get() const { return 2; }
 };
struct Three { 
  auto get() const { return 3; }
};
struct Four {
  auto get() const { return 4; }
 };

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;


std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> random_pick(0,3); // distribution in range [1, 6]

template <std::size_t N>
std::array<int, N> get_random_array() {
  std::array<int, N> item;
  for (int i = 0 ; i < N; i++)
    item[i] = random_pick(rng);
  return item;
}

template <typename T, std::size_t N>
std::array<T, N> get_random_objects(std::function<T(decltype(random_pick(rng)))> func) {
    std::array<T, N> a;
    std::generate(a.begin(), a.end(), [&] {
        return func(random_pick(rng));
    });
    return a;
}


static void TradeSpaceForPerformance(benchmark::State& state) {
    One one;
    Two two;
    Three three;
    Four four;

  int index = 0;

  auto ran_arr = get_random_array<50>();
  int r = 0;

  auto pick_randomly = [&] () {
    index = ran_arr[r++ % ran_arr.size()];
  };

  pick_randomly();


  for (auto _ : state) {

    int res;
    switch (index) {
      case 0:
        res = one.get();
        break;
      case 1:
        res = two.get();
        break;
      case 2:
        res = three.get();
        break;
      case 3:
        res = four.get();
        break;
    }
    
    benchmark::DoNotOptimize(index);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }


}
// Register the function as a benchmark
BENCHMARK(TradeSpaceForPerformance);


static void Virtual(benchmark::State& state) {

  struct Base {
    virtual int get() const noexcept = 0;
    virtual ~Base() {}
  };

  struct A final: public Base {
    int get()  const noexcept override { return 1; }
  };

  struct B final : public Base {
    int get() const noexcept override { return 2; }
  };

  struct C final : public Base {
    int get() const noexcept override { return 3; }
  };

  struct D final : public Base {
    int get() const noexcept override { return 4; }
  };

  Base* package = nullptr;
  int r = 0;
  auto packages = get_random_objects<Base*, 50>([&] (auto r) -> Base* {
          switch(r) {
              case 0: return new A;
              case 1: return new B;
              case 3: return new C;
              case 4: return new D;
              default: return new C;
          }
    });

  auto pick_randomly = [&] () {
    package = packages[r++ % packages.size()];
  };

  pick_randomly();

  for (auto _ : state) {

    int res = package->get();

    benchmark::DoNotOptimize(package);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }


  for (auto &i : packages)
    delete i;

}
BENCHMARK(Virtual);




static void FunctionPointerList(benchmark::State& state) {

    One one;
    Two two;
    Three three;
    Four four;
  using type = std::function<int()>;
  std::size_t index;

  auto packages = get_random_objects<type, 50>([&] (auto r) -> type {
        switch(r) {
        case 0: return std::bind(&One::get, one);
        case 1: return std::bind(&Two::get, two);
        case 2: return std::bind(&Three::get, three);
        case 3: return std::bind(&Four::get, four);
        default: return std::bind(&Three::get, three);
        }
    });
  int r = 0;

  auto pick_randomly = [&] () {
    index = r++ % packages.size();
  };


  pick_randomly();

  for (auto _ : state) {

    int res = packages[index]();

    benchmark::DoNotOptimize(index);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }

}
BENCHMARK(FunctionPointerList);



static void Index(benchmark::State& state) {

    One one;
    Two two;
    Three three;
    Four four;
  using type = std::variant<One, Two, Three, Four>;
  type* package = nullptr;

  auto packages = get_random_objects<type, 50>([&] (auto r) -> type {
        switch(r) {
            case 0: return one;
            case 1: return two;
            case 2: return three;
            case 3: return four;
            default: return three;
        }
    });
  int r = 0;

  auto pick_randomly = [&] () {
    package = &packages[r++ % packages.size()];
  };


  pick_randomly();

  for (auto _ : state) {

    int res;
    switch (package->index()) {
      case 0: 
        res = std::get<One>(*package).get();
        break;
      case 1:
        res = std::get<Two>(*package).get();
        break;
      case 2:
        res = std::get<Three>(*package).get();
        break;
      case 3:
        res = std::get<Four>(*package).get();
        break;
    }

    benchmark::DoNotOptimize(package);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }

}
BENCHMARK(Index);



static void GetIf(benchmark::State& state) {
    One one;
    Two two;
    Three three;
    Four four;
  using type = std::variant<One, Two, Three, Four>;
  type* package = nullptr;

  auto packages = get_random_objects<type, 50>([&] (auto r) -> type {
        switch(r) {
            case 0: return one;
            case 1: return two;
            case 2: return three;
            case 3: return four;
            default: return three;
        }
    });
  int r = 0;

  auto pick_randomly = [&] () {
    package = &packages[r++ % packages.size()];
  };

  pick_randomly();

  for (auto _ : state) {

    int res;
    if (auto item = std::get_if<One>(package)) {
      res = item->get();
    } else if (auto item = std::get_if<Two>(package)) {
      res = item->get();
    } else if (auto item = std::get_if<Three>(package)) {
      res = item->get();
    } else if (auto item = std::get_if<Four>(package)) {
      res = item->get();
    }

    benchmark::DoNotOptimize(package);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }
  

}
BENCHMARK(GetIf);

static void HoldsAlternative(benchmark::State& state) {
    One one;
    Two two;
    Three three;
    Four four;
  using type = std::variant<One, Two, Three, Four>;
  type* package = nullptr;

  auto packages = get_random_objects<type, 50>([&] (auto r) -> type {
        switch(r) {
            case 0: return one;
            case 1: return two;
            case 2: return three;
            case 3: return four;
            default: return three;
        }
    });
  int r = 0;

  auto pick_randomly = [&] () {
    package = &packages[r++ % packages.size()];
  };

  pick_randomly();

  for (auto _ : state) {

    int res;
    if (std::holds_alternative<One>(*package)) {
      res = std::get<One>(*package).get();
    } else if (std::holds_alternative<Two>(*package)) {
      res = std::get<Two>(*package).get();
    } else if (std::holds_alternative<Three>(*package)) {
      res = std::get<Three>(*package).get();
    } else if (std::holds_alternative<Four>(*package)) {
      res = std::get<Four>(*package).get();
    }

    benchmark::DoNotOptimize(package);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }

}
BENCHMARK(HoldsAlternative);


static void ConstexprVisitor(benchmark::State& state) {

    One one;
    Two two;
    Three three;
    Four four;
  using type = std::variant<One, Two, Three, Four>;
  type* package = nullptr;

  auto packages = get_random_objects<type, 50>([&] (auto r) -> type {
        switch(r) {
            case 0: return one;
            case 1: return two;
            case 2: return three;
            case 3: return four;
            default: return three;
        }
    });
  int r = 0;

  auto pick_randomly = [&] () {
    package = &packages[r++ % packages.size()];
  };

  pick_randomly();

  auto func = [] (auto const& ref) {
        using type = std::decay_t<decltype(ref)>;
        if constexpr (std::is_same<type, One>::value) {
            return ref.get();
        } else if constexpr (std::is_same<type, Two>::value) {
            return ref.get();
        } else if constexpr (std::is_same<type, Three>::value)  {
          return ref.get();
        } else if constexpr (std::is_same<type, Four>::value) {
            return ref.get();
        } else {
          return 0;
        }
    };

  for (auto _ : state) {

    auto res = std::visit(func, *package);

    benchmark::DoNotOptimize(package);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }

}
BENCHMARK(ConstexprVisitor);

static void StructVisitor(benchmark::State& state) {

  

  struct VisitPackage
  {
      auto operator()(One const& r) { return r.get(); }
      auto operator()(Two const& r) { return r.get(); }
      auto operator()(Three const& r) { return r.get(); }
      auto operator()(Four const& r) { return r.get(); }
  };

    One one;
    Two two;
    Three three;
    Four four;
  using type = std::variant<One, Two, Three, Four>;
  type* package = nullptr;

  auto packages = get_random_objects<type, 50>([&] (auto r) -> type {
        switch(r) {
            case 0: return one;
            case 1: return two;
            case 2: return three;
            case 3: return four;
            default: return three;
        }
    });
  int r = 0;

  auto pick_randomly = [&] () {
    package = &packages[r++ % packages.size()];
  };

  pick_randomly();

  auto vs = VisitPackage();

  for (auto _ : state) {

    auto res = std::visit(vs, *package);

    benchmark::DoNotOptimize(package);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }

}
BENCHMARK(StructVisitor);


static void Overload(benchmark::State& state) {


    One one;
    Two two;
    Three three;
    Four four;
  using type = std::variant<One, Two, Three, Four>;
  type* package = nullptr;

  auto packages = get_random_objects<type, 50>([&] (auto r) -> type {
        switch(r) {
            case 0: return one;
            case 1: return two;
            case 2: return three;
            case 3: return four;
            default: return three;
        }
    });
  int r = 0;

  auto pick_randomly = [&] () {
    package = &packages[r++ % packages.size()];
  };

  pick_randomly();

  auto ov = overload {
      [] (One const& r) { return r.get(); },
      [] (Two const& r) { return r.get(); },
      [] (Three const& r) { return r.get(); },
      [] (Four const& r) { return r.get(); }
    };

  for (auto _ : state) {

    auto res = std::visit(ov, *package);

  
    benchmark::DoNotOptimize(package);
    benchmark::DoNotOptimize(res);

    pick_randomly();
  }

}
BENCHMARK(Overload);


// BENCHMARK_MAIN();

