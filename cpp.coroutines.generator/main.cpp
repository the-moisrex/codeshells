#include <coroutine>
#include <generator>
#include <iostream>
#include <random>
#include <ranges>

using namespace std;

generator<int> randomer(int from = 0, int to = 100) {
  // Create a random number generator.
  std::mt19937 generator(12345);

  // Create a uniform distribution between 0 and 100.
  std::uniform_int_distribution<int> distribution(from, to);
  for (;;)
    co_yield distribution();
}

auto main() -> int {

  for (const int i : randomer() | views::take(10)) {
    cout << i << endl;
  }

  return 0;
}
