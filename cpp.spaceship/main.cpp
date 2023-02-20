#include <iostream>
#include <compare>
#include <string>

using namespace std;

struct A {
  string v;
  auto operator<=>(string i) const { return v <=> i; }
};

auto main() -> int {


  A a{.v = string{"nice"}};
  if (a == string{"nice"}) {
    cout << "works";
  } else {
    cout << "nope";
  }
  cout << endl;

  return 0;
}
