#include <atomic>
#include <concepts>
#include <iostream>

using namespace std;

template <typename T>
concept BooleanLike = requires(T val) {
                          val = true;
                          { !!val } noexcept -> same_as<bool>;
                      };

auto main() -> int {

    // https://godbolt.org/z/nxKz8fcE7
    // this will produce different result on GCC 12.2
    // but it's fixed on GCC 12.3
    cout << boolalpha;
    cout << BooleanLike<bool> << endl;
    cout << BooleanLike<atomic_bool> << endl;

    return 0;
}
