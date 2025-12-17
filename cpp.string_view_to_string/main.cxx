#include <string>
import standard;

#include <cstdint>
#include <print>

using namespace std;

template <typename CharT = char, typename CharTraits = std::char_traits<CharT>>
[[nodiscard]] constexpr std::basic_string<CharT, CharTraits>
operator+(std::basic_string_view<CharT, CharTraits> const str) {
    return {str.data(), str.size()};
}

int main() {

    std::string_view strv = "hello world";
    std::string str = +strv;

    println("Number: {}", str);

    return 0;
}

// view ./build/optimizations.txt
