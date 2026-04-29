import standard;

#include <cstdint>
#include <utility>
#include <print>

using namespace std;
namespace stl {
    using namespace std;
}

/**
 * Force implicitly convert and use the explicit conversion.
 */
template <typename From>
struct [[nodiscard]] implicitly_explicit_convert {
    static_assert(stl::is_reference_v<From>, "It must be a reference.");
    From _ref;

    template <typename To>
        requires(!stl::is_reference_v<To> && stl::constructible_from<To, From>)
    [[nodiscard]] explicit(false) constexpr operator To() const {
        // The paranthesis here are the magic part
        return To(static_cast<From&&>(_ref));
    }
};

template <typename From>
implicitly_explicit_convert(From&&) -> implicitly_explicit_convert<From&&>;

template <typename T>
concept has_proxy_operators = requires(stl::remove_cvref_t<T> obj) {
    { enable_proxy_operators(obj) } -> stl::same_as<bool>;
    // requires enable_proxy_operators(obj);
};

/*
 * Use this operator for example on string_view to like `string = +strv_obj` to
 * automatically use the explicit constructor of the string.
 */
template <typename T>
    requires has_proxy_operators<T>
constexpr auto operator+(T&& obj) noexcept {
    return implicitly_explicit_convert<T&&>{std::forward<T>(obj)};
}

template <typename T>
    requires (stl::is_enum_v<stl::remove_cvref_t<T>>)
constexpr auto operator+(T&& obj) noexcept {
    return stl::to_underlying(obj);
}

enum struct objects { toys, phones, keyboard};


int main() {

    int res = +objects::phones;
    println("Number: {}", res);

    return 0;
}

// view ./build/optimizations.txt
