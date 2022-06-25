#include <iostream>
#include <string_view>

using namespace std;

struct options {
  string_view values = "value";
  bool one = false;
  bool two = true;
  bool three = false;
};

struct wow {
  options opts;

  wow(std::string_view str, options o = {}) : opts{o} {
    cout << str << endl;
  }
};

auto main() -> int {

  wow data("not wow");
  cout << data.opts.values << endl;

  return 0;
}

// it works here, why doesn't it work in my project then?


