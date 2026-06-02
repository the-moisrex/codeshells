import standard;

// Reference example of the pattern
// See: https://medium.com/@felixolivierdumas/exotic-crtp-rethinking-static-polymorphism-with-c-23-89f9e75e8ffd

#include <iostream>
#include <type_traits>
#include <utility>

namespace exotic {

template<typename From>
struct crtp_access : From {};

template<typename T>
constexpr decltype(auto) as_crtp(T&& obj) noexcept {
    using crtp_access_t = crtp_access<std::remove_cvref_t<T>>;
    return static_cast<crtp_access_t&&>(obj);
}

}

template <typename T>
struct Base {
    decltype(auto) self() noexcept {
        return exotic::as_crtp(static_cast<T&&>(*this));
    }

    void interface() {
        self().implementation();
    }
};

struct Derived : Base<Derived> {
    int a = 20;


    void implementation(this exotic::crtp_access<Derived> self) {
        std::cout << "Derived implementation: " << self.a << std::endl;
    }
};

int main() {
    Derived d;
    d.a = 23;
    d.interface(); // perfectly works

    // d.implementation(); -> doesn't work, Derived only allows .interface()
}

