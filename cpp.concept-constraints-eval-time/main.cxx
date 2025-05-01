#include <type_traits>
import standard;

#include <cstdint>
#include <print>

using namespace std;

template <integral T>
static constexpr bool val = is_same_v<T, int>;

template <typename T>
concept valConcept = integral<T> && is_same_v<T, int>;

template <typename T>
concept canwe = val<T>;

// this works:
static_assert(!canwe<double>);

// also converting it to concept works too:
static_assert(!valConcept<double>);

// this doesn't:
// static_assert(!requires {
//     typename val<double>;
// });

// So will this:
template <typename T>
static constexpr bool canweBool = requires { requires val<T>; };
static_assert(!canweBool<double>);


int main() {

    println("Number: {}", 2);

    return 0;
}

// view ./build/optimizations.txt
