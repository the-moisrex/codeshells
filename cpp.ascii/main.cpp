#include <bitset>
#include <iomanip>
#include <iostream>

using namespace std;

auto main() -> int {

  for (char ch = 0; ch != 127; ++ch) {
    cout << dec << setw(3) << static_cast<int>(ch) << " "
         << (ch >= 32 ? ch : '?') << " " << bitset<8>(ch) << " 0x" << hex
         << static_cast<int>(ch) << endl;
  }

  return 0;
}
