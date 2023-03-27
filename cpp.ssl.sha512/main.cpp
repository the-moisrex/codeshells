#include <iostream>
#include <random>
#include <openssl/sha.h>
#include <algorithm>

using namespace std;

auto main() -> int {

    string str = "Hello world, this seems amazing";
    str = str + str + str + str + str + str;
    random_device d;
    mt19937 g{d()};
    shuffle(str.begin(), str.end(), g);
    cout << str << endl;

    string hashed;
    hashed.resize(SHA512_DIGEST_LENGTH);
    SHA512(reinterpret_cast<const unsigned char*>(str.data()),
            str.size(),
            reinterpret_cast<unsigned char*>(hashed.data()));

    // cout << hashed << endl;
    for ( char c : hashed) {
        cout << hex << (int)c;
    }

  return 0;
}
