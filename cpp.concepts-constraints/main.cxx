import standard;

#include <cstdint>
#include <print>

using namespace std;

template <typename T, typename U>
concept test = integral<T> && std::same_as<U, string_view>;

template <typename T, typename U>
struct test2_type {
    static constexpr bool value = false;
};

template <typename T, typename U>
    requires(!integral<T> && integral<U>)
struct test2_type<T, U> {
    // no ::value will cause a blow up
};

template <integral T, typename U>
    requires std::same_as<U, string_view>
struct test2_type<T, U> {
    static constexpr bool value = true;
};

template <typename T, typename U>
concept test2 = test2_type<T, U>::value;

int main() {

    println("Test: {}", test<string_view, int>);
    println("Test: {}", test<int, string_view>);

    println("Test2: {}", test<string_view, int>);
    println("Test2: {}", test<int, string_view>);

    // this should blow up which is what we want:
    // println("Test3: {}", test2_type<string_view, int>::value);
    println("Test3: {}", test2_type<int, string_view>::value);

    return 0;
}

// view ./build/optimizations.txt
