#include <iostream>
#include <concepts>
#include <type_traits>
#include <any>

using namespace std;

struct one_t {
    constexpr void operator()(int) {}
};

struct two_t {
    constexpr void operator()(int&) {}
};

struct converter {
    any data;

    template <typename T>
    constexpr operator T() {
        return any_cast<T>(data);
    }

    template <typename T>
    constexpr operator T&() {
        return any_cast<T>(data);
    }
};

auto main() -> int {

    cout << boolalpha;

    cout << is_invocable_v<one_t, int&> << endl;
    cout << is_invocable_v<two_t, int&> << endl;
    cout << is_invocable_v<one_t, converter> << endl;
    cout << is_invocable_v<two_t, converter> << endl;
    cout << is_invocable_v<two_t, converter&> << endl;

  return 0;
}
