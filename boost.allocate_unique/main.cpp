#include "pch.h"
#include <boost/smart_ptr/allocate_unique.hpp>

using namespace std;

auto main() -> int {

    allocator<int> int_alloc;

    auto data = boost::allocate_unique<string>(int_alloc);

    *data = "nice";

    cout << *data << endl;

  return 0;
}
