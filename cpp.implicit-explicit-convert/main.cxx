import standard;

#include <concepts>
#include <cstdint>
#include <print>
#include <type_traits>
#include <utility>

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
    using from_type = stl::remove_cvref_t<From>;
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

/*
 * Use this operator for example on string_view to like `string = +strv_obj` to
 * automatically use the explicit constructor of the string.
 */
template <typename T>
    requires(!requires(T&& obj) { +std::forward<T>(obj); }) &&
            (stl::is_class_v<stl::remove_cvref_t<T>> ||
             stl::is_union_v<stl::remove_cvref_t<T>>)
constexpr auto operator+(T&& obj) noexcept {
    return implicitly_explicit_convert<T&&>{std::forward<T>(obj)};
}

template <typename T>
    requires(stl::is_enum_v<T>)
[[nodiscard]] constexpr auto operator+(T obj) noexcept {
    return stl::to_underlying(obj);
}

enum struct Objects { toy, phone, paper };

int main() {

    int res = +Objects::phone;
    println("Number: {}", res);

    return 0;
}

// view ./build/optimizations.txt
