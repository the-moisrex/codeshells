#include <iostream>

using namespace std;

template <typename A> struct B {

  [[no_unique_address]] struct one_type {
    A *ref() { return reinterpret_cast<A *>(this); }
    A &operator=(int v) {
      ref()->val = v + 1;
      return *ref();
    }
  } one;

  [[no_unique_address]] struct two_type {
    A *ref() { return reinterpret_cast<A *>(this); }
    A &operator=(int v) {
      ref()->val = v + 2;
      return *ref();
    }
  } two;
};
struct A : B<A> {
  int val;
};

auto main() -> int {

  A a;
  a.one = 1;
  cout << a.val << endl;
  a.two = 1;
  cout << a.val << endl;

  return 0;
}
