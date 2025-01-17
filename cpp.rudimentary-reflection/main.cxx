import standard;

// from: https://x.com/ajay9470/status/1880361150417870993

#include <iostream>
#include <utility>

template <typename T>
struct base_type {};

template <>
struct base_type<int> {
    static std::string get_type() { return "int"; }
};

template <>
struct base_type<float> {
    static std::string get_type() { return "float"; }
};

// write more specializations for primitive and pointers and arrays, not
// important here

struct ubiq_lref {
    std::string& out;
    std::string dummy;
    ubiq_lref(const int& _) : out(dummy) {}
    ubiq_lref(std::string& x, const int& _) : out(x) {}

    template <typename T>
    operator T() {
        out += base_type<T>::get_type() + ", ";
        return T();
    }
};

template <typename T, int... ints>
struct reflect {
    static std::string get_members() {
        std::string s = "{";
        T{ubiq_lref(s, ints)...};
        s += "}";
        return s;
    }
};

template <typename T, int... ints>
auto get_str(int) -> typename std::enable_if<
    std::is_same<decltype(T{ubiq_lref(ints)...}), T>::value,
    std::string>::type {
    return reflect<T, ints...>::get_members();
}

template <typename T, int a, int... ints>
auto get_str(long) -> std::string {
    return get_str<T, ints...>(0);
}

template <typename T, int... ints>
std::string get_members(std::integer_sequence<int, ints...> _) {
    return get_str<T, ints...>(0);
}

struct foo {
    int x;
    float y;
    int z;
};

int main() {
    // your code goes here
    std::cout << get_members<foo>(
                     std::make_integer_sequence<int, (int)sizeof(foo)>())
              << std::endl;
    return 0;
}
