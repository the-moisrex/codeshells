#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

using namespace std;

vector<int> gvec() { return {1, 2, 3, 4, 5, 6, 7}; }

auto main() -> int {
  auto const data = gvec() | views::drop(1) | views::take(5);
  bool const res = ranges::all_of(data, [](const int i) { return i < 5; });

  cout << boolalpha;
  cout << "Result: " << res << endl;
  cout << data.front();
  for (const int i : ranges::ref_view(data) | views::drop(1)) {
    cout << ", " << i;
  }
  cout << endl;

  return 0;
}
