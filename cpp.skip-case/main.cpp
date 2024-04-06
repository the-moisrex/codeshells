#include <iostream>

using namespace std;

#define skip if (false)

auto main() -> int {

    switch (0) {
    skip case 0: { cout << "hello"; }
    skip case 1: { cout << "world"; }
        cout << endl;
        break;
    }

    return 0;
}
