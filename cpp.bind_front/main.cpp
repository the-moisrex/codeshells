#include <functional>
#include <iostream>

int minus(int a, int b) { return a - b; }

struct S {
    int val;
    int minus(int arg) const noexcept { return val - arg; }
};

int main() {
    auto fifty_minus = std::bind_front(minus, 50);
    std::cout << fifty_minus(3) << '\n'; // equivalent to `minus(50, 3)`

    auto member_minus = std::bind_front(&S::minus, S{50});
    std::cout << member_minus(3)
              << '\n'; // equivalent to `S tmp{50}; tmp.minus(3)`

    // noexcept-specification is preserved!
    static_assert(!noexcept(fifty_minus(3)));
    static_assert(noexcept(member_minus(3)));

    // binding of a lambda
    auto plus = [](int a, int b) { return a + b; };
    auto forty_plus = std::bind_front(plus, 40);
    std::cout << forty_plus(7) << '\n'; // equivalent to `plus(40, 7)`

#ifdef __cpp_lib_bind_back
    auto madd = [](int a, int b, int c) { return a * b + c; };
    auto mul_plus_seven = std::bind_back(madd, 7);
    std::cout << mul_plus_seven(4, 10)
              << '\n'; // equivalent to `madd(4, 10, 7)`
#endif
}
