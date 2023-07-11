#include <iostream>
#include <version>

using namespace std;
#if __cpp_multidimensional_subscript
void nice() { cout << "Compiler support" << endl; }
#else
void nice() { cout << "does not support" << endl; }
#endif

auto main() -> int {

  nice();

  return 0;
}
