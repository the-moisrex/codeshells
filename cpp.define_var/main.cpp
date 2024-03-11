#include <iostream>

using namespace std;

#define define_var(name, ...) auto name = [ var_name = #name, __VA_ARGS__ ]

auto main() -> int {

  define_var(count, i = 0)() mutable {
    cout << var_name << " " << ++i << endl;
  };

  count();
  count();

  return 0;
}
