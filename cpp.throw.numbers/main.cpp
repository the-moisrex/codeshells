#include <iostream>

using namespace std;

auto main() -> int {

    try {
        throw 1;
    } catch (int a) {
        cout << "int: " << a << endl;
    }

    return 0;
}
