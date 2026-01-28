#include <string>
import standard;

#include <cstdint>
#include <print>

using namespace std;

// library code:
namespace myproject {

    template <typename T, int priority = 1>
    struct default_string_type : default_string_type<T, priority - 1> {};

    template <typename T>
    struct default_string_type<T, 0> {
        using char_type = char;
        using char_traits = std::char_traits<char_type>;
        using allocator_type = std::allocator<char_type>;
        using type = std::basic_string<char_type, char_traits, allocator_type>;
    };

    template <typename T = void>
    using string = typename default_string_type<T>::type;

    // The library needs to use it:
    struct test {
        string<> str;
    };

} // namespace myproject

/// User code:
namespace myproject {
    template <>
    struct default_string_type<void> {
        using type = std::string;
    };
} // namespace myproject

int main() { return 0; }

// view ./build/optimizations.txt
