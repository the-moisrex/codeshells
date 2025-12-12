import standard;

#include <cstdint>
#include <print>

using namespace std;

/**
 * Usage:
 * run([] (this auto& self, Context auto& ctx) {
 *   // do anything you want
 * })
 */
template <typename... Bases>
struct run : Bases... {
    static_assert((std::copyable<Bases> && ...), "The bases must be copyable");

    template <typename... Args>
    explicit(false) consteval run(Args&&... args)
        : Bases{std::forward<Args>(args)...}... {}

    consteval run() = default;
    constexpr run(run const&) = default;
    constexpr run(run&&) noexcept = default;
    constexpr run& operator=(run const&) = default;
    constexpr run& operator=(run&&) noexcept = default;
    constexpr ~run() = default;


    int value = 0;
};

template <typename... Bases>
run(Bases&&...) -> run<Bases...>;

int main() {

    auto runner = run([] (this auto& self) {
        self.value = 2;
    });

    println("Number: {}", runner.value);
    runner();
    println("Number: {}", runner.value);

    return 0;
}

// view ./build/optimizations.txt
