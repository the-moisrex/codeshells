#include <iostream>
#include <array>
#include <type_traits>

template <typename... Char, typename = typename std::enable_if<(true && ... && std::is_same_v<Char, char>), void>::type>
constexpr auto charset(Char... chars) noexcept {
        return std::array<char, sizeof...(chars)>{chars...};
}


using namespace std;

auto main() -> int {

    constexpr auto a = charset('a', 'b', 'c');
    // constexpr auto a = charset("one", "two", "three");

    for (char const& i : a)
        cout << i << endl;

  return 0;
}
