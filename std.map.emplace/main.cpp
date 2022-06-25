#include <iostream>
#include <map>


using namespace std;

auto main() -> int {

  map<int, int> data;
  data.emplace(1, 1);
  data.emplace(1, 2);
  auto two = data.find(1);

  cout << two->second << endl;

  return 0;
}
