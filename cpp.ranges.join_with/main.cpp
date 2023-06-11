#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

int main() {
    using namespace std::literals;

    std::vector v{"This"sv, "is"sv, "a"sv, "test."sv};
    auto joined = v | std::views::join_with(' ');

    for (int i = 0; auto c : joined)
        std::cout << ++i << ". " << c << std::endl;
    std::cout << '\n';
}
