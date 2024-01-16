#include <iostream>

using namespace std;

auto main() -> int {

    char v = 'A';
    char c = ((v + (128 - 'A')) ^ (v + (128 - 'Z' - 1)) & 0x80) >> 2;
    v ^= c;
    cout << static_cast<int>(c) << endl;
    cout << v << endl;

    return 0;
}
