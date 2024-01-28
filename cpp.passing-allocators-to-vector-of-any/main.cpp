#include <any>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using namespace std;

struct test {
    vector<any> anything;
    vector<any> empty_anything;

    test()
        : anything{allocator<any>()},      // allocates 1 std::any
          empty_anything(allocator<any>()) // passes the allocator to the vector
    {}
};

auto main() -> int {

    test obj;
    cout << obj.anything.size() << endl;
    cout << obj.empty_anything.size() << endl;

    return 0;
}
