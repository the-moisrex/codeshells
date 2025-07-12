import standard;

#include <cstdint>
#include <print>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

using namespace std;

namespace {
    template <typename T>
    struct construct_it_from {
        template <typename... Args>
        [[nodiscard]] constexpr T
        operator()(Args&&... args) noexcept(std::constructible_from<T>) {
            return T{std::forward<Args>(args)...};
        }
    };

    template <typename T>
    [[nodiscard]] constexpr auto transform_to() {
        return std::views::transform(construct_it_from<T>{});
    }

    template <typename Inp, typename T>
        requires(std::is_invocable_v<T, construct_it_from<Inp>>)
    [[nodiscard]] constexpr auto into(T&& obj) {
        return std::forward<T>(obj)(construct_it_from<Inp>{});
    }
} // namespace

int main(int const argc, char** argv) {
    using std::ranges::to;
    using std::views::drop;
    using std::views::transform;

    auto args = std::span{argv, argv + argc} | drop(1) |
                transform_to<std::string_view>();

    println("Size: {}", args.size());
    for (auto const arg : args) {
        println("Number: {}", arg);
    }

    return 0;
}

// view ./build/optimizations.txt
