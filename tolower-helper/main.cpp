#include <iostream>

using namespace std;

auto main() -> int {

  cout << boolalpha;
  //  for (char a = 0b01100000; a != 0b01111111; ++a) {
  //    cout << "'" <<  a << "'" << " " << static_cast<bool>(islower(a)) << endl;
  //  }
  //
  //  for (char a = 'A' - 10; a != 'Z' + 10; ++a) {
  //    cout << "'" <<  a << "'" << " " << static_cast<bool>(isupper(a)) << endl;
  //  }

  for (char a = 0; a < 127; ++a) {
    cout << "'" << a << "'" << " "
         << ((a & 0b01100000) == 0b01000000) << " "
         << (((a & 0b01100000) == 0b01000000) && ((a & 0b01111000) != 0b01111000)) << " "
         << static_cast<char>((a | 0b00100000)) << " "
         << static_cast<char>((a & 0b01011111))
         << endl;
  }


  return 0;
}
