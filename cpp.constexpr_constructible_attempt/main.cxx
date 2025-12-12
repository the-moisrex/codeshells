import standard;

#include <cstdint>
#include <print>

using namespace std;

template <typename T>
concept constexpr_constructible = requires {
    requires [] consteval {
        [[maybe_unused]] T t{};
        return true;
    }(); // If it fails, T cannot be compile time constructed.
};

// template<typename T>
// concept constexpr_constructible = []<typename U>() consteval -> bool {
//     if constexpr (requires { U{}; }) {
//         U u{};        // this is evaluated in a consteval context
//         return true;
//     } else {
//         return false;
//     }
// }.template operator()<T>();

struct A {
    int* a;

    // `= default` implies constexpr
    A() {
       a = new int(0);
    }

};

struct B{};

int main() {

    println("Number: {}", constexpr_constructible<A>);

    return 0;
}

// view ./build/optimizations.txt
