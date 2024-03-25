#include <iostream>

// https://www.atnnn.com/p/operator-larrow/
// Sometimes you have a pointer to a class, and you want to invoke a method. You
// can use the -> operator for that.

// So what do you do when you have a pointer to a method, and want to invoke it
// on a class? Use the <- operator!

template <class T>
struct larrow {
    larrow(T* a_) : a(a_) {}
    T* a;
};

template <class T, class R>
R operator<(R (T::*f)(), larrow<T> it) {
    return (it.a->*f)();
}

template <class T>
larrow<T> operator-(T& a) {
    return larrow<T>(&a);
}

struct C {
    void f() { std::cout << "foo\n"; }
};

int main() {
    C x;
    (&C::f) < -x;
}
