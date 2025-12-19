import standard;

#include <type_traits>
#include <utility>

template <typename From>
struct implicitly_explicit_convert {
    static_assert(std::is_reference_v<From>);
    From _ref;
    template <typename To>
    constexpr operator To() const {
        static_assert(!std::is_reference_v<To>);
        return To(static_cast<From&&>(_ref));
    }
};

template <typename From>
implicitly_explicit_convert(From&&) -> implicitly_explicit_convert<From&&>;

// I don't condone this btw but you do you
template <typename T>
[[nodiscard]] constexpr auto operator+(T&& obj) noexcept {
    return implicitly_explicit_convert<T&&>{std::forward<T>(obj)};
}

struct my_explicitly_constructible {
    int _v;
    explicit my_explicitly_constructible(int v) : _v(v) {}
};

struct my_explicitly_convertible {
    int _v;
    explicit operator int() const { return _v; }
};

void fn(int&&) {}

int main() {
    // my_explicitly_constructible a = 42;
    my_explicitly_constructible b = implicitly_explicit_convert{42};

    // int c = my_explicitly_convertible{42};
    int d = implicitly_explicit_convert{my_explicitly_convertible{42}};

    // This case properly creates an implicitly_explicit_convert<int&>
    int my_lvalue = 42;
    my_explicitly_constructible f = implicitly_explicit_convert{my_lvalue};

    // Cases that I thought were problematic, but I guess conversion operators
    // follow slightly different rules for deduction.

    // It seems like the template conversion operator doesn't deduce this to a
    // reference (thankfully)
    int&& g = implicitly_explicit_convert{my_explicitly_convertible{42}};

    // This one doesn't either, this is great!
    fn(implicitly_explicit_convert{my_explicitly_convertible{42}});

    return d + g + f._v + b._v;
}
