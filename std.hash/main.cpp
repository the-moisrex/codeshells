#include <iostream>
#include <functional>
#include <string>

using namespace std;

auto main() -> int {

  std::string str = "hello world";
  hash<string> hash_str;
  cout << hash_str(str) << endl;

  cout << hash<long long>()(rand()) << endl;

  return 0;
}
