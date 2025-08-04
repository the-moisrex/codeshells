import standard;

#include <cstdint>
#include <print>
#include <climits>

using namespace std;

struct basic_abs2rel {
    using value_type = int;
};

constexpr basic_abs2rel::value_type states_loc   = sizeof(basic_abs2rel::value_type) * CHAR_BIT - 3;
constexpr basic_abs2rel::value_type x_bit_loc    = states_loc;
constexpr basic_abs2rel::value_type y_bit_loc    = states_loc + 1;
constexpr basic_abs2rel::value_type x_init_state = 0b1 << x_bit_loc;
constexpr basic_abs2rel::value_type y_init_state = 0b1 << y_bit_loc;



int main() {

    println("stateless location: {}", states_loc);
    println("x loc: {}", x_bit_loc);
    println("y loc: {}", y_bit_loc);
    println("x init: 0x{:x}", x_init_state);
    println("y init: 0x{:x}", y_init_state);

    return 0;
}

// view ./build/optimizations.txt
