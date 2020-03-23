#include <iostream>
#include <cmath>
using namespace std;

template <typename T>
T constexpr pi = std::acos(-T(1));

auto main() -> int {

	cout << pi<float> << endl;
	cout << pi<double> << endl;
	cout << pi<long double> << endl;
	cout << pi<int> << endl;

  return 0;
}
