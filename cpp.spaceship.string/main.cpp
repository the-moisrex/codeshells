#include <compare>
#include <iostream>

using namespace std;

auto main() -> int {

  string one = "one";
  string two = "two";

  if (!is_eq(one <=> two)) {
    cout << "not eq" << endl;
  }

  return 0;
}
