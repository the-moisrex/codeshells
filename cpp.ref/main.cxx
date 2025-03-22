#include <type_traits>
import standard;

#include <cstdint>
#include <type_traits>
#include <print>

template <typename T>
struct ref {
    T* ptr;

    constexpr ref(T& obj_ref) noexcept : ptr{std::addressof(obj_ref)} {}

    constexpr operator T&() const noexcept {
        return *ptr;
    }

    constexpr T* operator->() const noexcept {
        return ptr;
    }

    constexpr T& operator*() const noexcept {
        return *ptr;
    }

    constexpr operator T() = delete
#ifdef __cpp_deleted_function
        ("You cannot copy this")
#endif
    ;
};

std::string str = "test";

decltype(auto) test() {
    return ref{str};
}

int main() {

    auto test_str = test();

    std::println("{}", *test_str);

  return 0;
}
