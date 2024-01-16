#include <iostream>

using namespace std;

auto main() -> int {

  // good article explaining this:
  // http://0x80.pl/notesen/2023-03-06-swar-find-any.html
  //
  // Soruce:
  // https://godbolt.org/z/qdnvTP3s3

  static constexpr char Ap = (128 - 'A');
  static constexpr char Zp = (128 - 'Z' - 1);

  cout << "Ap: " << static_cast<int>(Ap) << endl;
  cout << "Zp: " << static_cast<int>(Zp) << endl;

  char v = 'A';

  cout << "v: " << static_cast<int>(v) << endl;
  cout << "v + Ap: " << static_cast<int>((v + Ap)) << endl;
  cout << "v + Zp: " << static_cast<int>((v + Zp)) << endl;
  cout << "^: " << static_cast<int>((v + Ap) ^ (v + Zp)) << endl;
  cout << "& 0x80: " << static_cast<int>(((v + Ap) ^ (v + Zp)) & 0x80) << endl;
  cout << ">> 2: " << static_cast<int>((((v + Ap) ^ (v + Zp)) & 0x80) >> 2)
       << endl;

  char c = (((v + Ap) ^ (v + Zp)) & 0x80) >> 2;
  cout << "v(int): " << static_cast<int>(v) << endl;
  v ^= c;
  cout << static_cast<int>(c) << " "
       << "0x" << std::hex << static_cast<int>(c) << endl;
  cout << v << endl;
  cout << "v(int): " << std::dec << static_cast<int>(v) << " "
       << "0x" << std::hex << static_cast<int>(v) << endl;
  return 0;
}
