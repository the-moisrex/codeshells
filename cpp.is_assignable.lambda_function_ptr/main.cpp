#include <iostream>

using namespace std;

struct mylambda {
    constexpr int operator()(char, double) const noexcept { return 23; }
};

auto main() -> int {

    auto lambda = [](char, double) { return 0; };
    using signature = int (*&)(char, double);
    static_assert(is_assignable<signature, decltype(lambda)>::value, "!");
    static_assert(
        is_assignable<signature, remove_cvref_t<decltype(lambda)>>::value, "!");
    static_assert(is_assignable<signature, mylambda>::value, "!");

    return 0;
}
