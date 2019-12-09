#include <iostream>
#include "headers.h"

using namespace std;
using namespace webpp;

class cgi {
};

int main() {

    headers<cgi> h;
    h.end();
    h.begin();
//    for (auto &c : h) {
//        cout << c << endl;
//    }


    return 0;
}
