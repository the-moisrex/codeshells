import standard;

#include <cstdint>
#include <iostream>
#include <print>

using namespace std;

// from: https://x.com/supahvee1234/status/1999899526883053921?s=20

// concept to check if type T has a constexpr constructor
// and thus can be used in constexpr contexts, in another words,
// the type T can be constructed in compiletime.
// template <typename T>
// concept constexpr_constructible = requires { []<T U = T{}>{}(); };

// template <typename T>
// concept constexpr_constructible =  requires { std::array<int, (T{}, 1)>{}; };


// template <typename T, typename ...Args>
// concept constexpr_constructible = requires { []<T U = T{std::declval<Args>()...}>{}(); };

// template <typename T>
// concept constexpr_constructible = requires { []<T={}>{}(); };


// template <typename T, typename ...Args>
// concept constexpr_constructible = requires { []<int=(T{Args{}...}, 0)>{}(); };

template <typename T, typename ...Args>
concept constexpr_constructible = requires { []<T = {Args{}...}>{}(); };



template <constexpr_constructible T>
void showValue(T obj) {
    std::cout << "Value of obj: " << obj.get() << std::endl;
}

struct A {
    int* p;
    A() {
        p = new int();
    }
};

struct B {
    consteval B() = default;
    B(B const&) = delete;
    constexpr ~B() { }
};

static_assert(constexpr_constructible<int>);
static_assert(constexpr_constructible<B>);
static_assert(!constexpr_constructible<A>);
static_assert(!constexpr_constructible<int&>);
static_assert(constexpr_constructible<int, int>);

int main() { return 0; }

// view ./build/optimizations.txt
