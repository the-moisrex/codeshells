import standard;

#include <generator>
#include <print>
#include <utility>

using namespace std;
using namespace std::views;

std::generator<int> fib() {
  int i = 0;
  int j = 1;
  for (;;) {
    co_yield i = std::exchange(j, i + j);
  }
}

int main() {

  for (const auto i : fib() | take(20)) {
    println("{}", i);
  }

  return 0;
}
