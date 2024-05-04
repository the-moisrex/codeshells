#include <absl/numeric/int128.h>
#include <iostream>

using namespace std;

auto main() -> int {

    absl::uint128 a = absl::MakeUint128(6000000000000005, 100000000000);

    cout << a << endl;

    return 0;
}
