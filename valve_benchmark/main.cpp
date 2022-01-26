#include <benchmark/benchmark.h>

#include <string_view>
#include <type_traits>
#include <string>
#include <random>
#include <array>
#include <functional>

template <typename Interface>
struct request_t {
    std::string r;
    auto request_method() const { return r; }
};

    enum class logical_operators { AND, OR, XOR };

    template <typename NextValve>
    struct basic_valve {
        using next_valve_type = std::remove_reference_t<std::remove_cv_t<NextValve>>;

        next_valve_type next;
        logical_operators op;

        constexpr basic_valve(next_valve_type&& _next, logical_operators op) noexcept
            : next(std::move(_next)), op(op) {}

        constexpr basic_valve(next_valve_type const& _next, logical_operators op) noexcept
            : next(_next), op(op) {}


        constexpr basic_valve(basic_valve const& v) noexcept = default;
        constexpr basic_valve(basic_valve&& v) noexcept = default;

        constexpr basic_valve& operator=(basic_valve const& v) noexcept = default;
        constexpr basic_valve& operator=(basic_valve&&) noexcept = default;

    };

    template <>
    struct basic_valve<void> {
    };

    template <typename ValveType, typename NextValve = void>
    class valve : public basic_valve<NextValve>, public ValveType {
      public:
        using type = ValveType;
        using next_valve_type = NextValve;


        using ValveType::ValveType;
        using basic_valve<NextValve>::basic_valve;
        constexpr valve() noexcept = default;

        /**
         * @tparam NewValveType
         * @param valve
         */
        template <typename NewValve>
        [[nodiscard]] constexpr auto
        set_next(NewValve&& v,
                 logical_operators the_op) const noexcept {

            if constexpr (std::is_void_v<next_valve_type>) {
                // this part will only execute when the "next_valve_type" is
                // void

                // the first way (A<X, void> and B<Y, void> === A<X, B<Y, void>>
                return valve<ValveType, NewValve>(
                    std::forward<NewValve>(v), the_op);
            } else {
                // this means this function has a "next" valve already,
                // so it goes to the next's next valve
                // this way we recursively create a valve type and return it.
                auto n = basic_valve<NextValve>::next.set_next(
                    v, the_op);
                return valve<ValveType, decltype(n)>{
                    n, this->op};
            }
        }

        template <typename NewValve>
        [[nodiscard]] constexpr auto
        operator&&(NewValve&& v) const noexcept {
            return set_next(std::forward<NewValve>(v),
                            logical_operators::AND);
        }

        template <typename NewValve>
        [[nodiscard]] constexpr auto
        operator&(NewValve&& v) const noexcept {
            return set_next(std::forward<NewValve>(v),
                            logical_operators::AND);
        }

        template <typename NewValve>
        [[nodiscard]] constexpr auto
        operator||(NewValve&& v) const noexcept {
            return set_next(std::forward<NewValve>(v),
                            logical_operators::OR);
        }

        template <typename NewValve>
        [[nodiscard]] constexpr auto
        operator|(NewValve&& v) const noexcept {
            return set_next(std::forward<NewValve>(v),
                            logical_operators::OR);
        }

        template <typename NewValve>
        [[nodiscard]] constexpr auto
        operator^(NewValve&& v) const noexcept {
            return set_next(std::forward<NewValve>(v),
                            logical_operators::XOR);
        }

        template <typename Interface>
        [[nodiscard]] bool operator()(request_t<Interface> const& req) const
            noexcept {
            if constexpr (std::is_void_v<NextValve>) {
                return ValveType::operator()(req);
            } else {
                switch (basic_valve<NextValve>::op) {
                case logical_operators::AND:
                    return ValveType::operator()(req) &&
                           basic_valve<NextValve>::next.operator()(req);
                case logical_operators::OR:
                    return ValveType::operator()(req) ||
                           basic_valve<NextValve>::next.operator()(req);
                case logical_operators::XOR:
                    return ValveType::operator()(req) ^
                           basic_valve<NextValve>::next.operator()(req);
                default:
                    return false;
                }
            }
        }
    };

    

    struct method_condition {
      private:
        std::string_view method_string;

      public:
        constexpr method_condition(std::string_view str) noexcept
            : method_string(str) {}
        constexpr method_condition() noexcept = default;

        template <typename Interface>
        [[nodiscard]] bool operator()(request_t<Interface> const& req) const noexcept {
            return req.request_method() == method_string;
        }
    };

    struct method : public valve<method_condition> {
        using valve<method_condition>::valve;
    };

    struct empty_condition {
        template <typename Interface>
        [[nodiscard]] constexpr bool operator()(request_t<Interface> const& /* req */) const
            noexcept {
            return true;
        }
    };

    struct empty_t : public valve<empty_condition> {};
    constexpr empty_t empty;




std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> random_pick(0,1); // distribution in range [1, 6]

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

auto rands = get_random_objects<request_t<void>, 50>([] (auto i) {
			auto c = i == 0 ? "PUT" : "GET";

			    return request_t<void>{
				 .r = c
				};
		});



static void Empty(benchmark::State& state) {

    std::size_t i = 0;
  for (auto _ : state) {


    auto res = rands[i++ % rands.size()].r == "PUT";
    benchmark::DoNotOptimize(res);
  }

}
// Register the function as a benchmark
BENCHMARK(Empty);


static void valve_version(benchmark::State& state) {

	
    constexpr auto con = method("PUT");

    std::size_t i = 0;
  for (auto _ : state) {

    auto res = con(rands[i++ % rands.size()]);

    benchmark::DoNotOptimize(res);
  }

}
// Register the function as a benchmark
BENCHMARK(valve_version);

// BENCHMARK_MAIN();
