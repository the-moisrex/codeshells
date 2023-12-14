#include <algorithm>
#include <vector>
#include <array>
#include <iostream>
#include <memory_resource>

using namespace std;

// https://godbolt.org/z/fxxMxePzf

void vec()  {
    array<byte, 100> stack{};
    pmr::monotonic_buffer_resource res{stack.data(),
                                       stack.size() * sizeof(std::byte)};
    pmr::polymorphic_allocator<byte> alloc{&res};

    auto size = 90;
    pmr::vector<byte> data{alloc}; 
    data.resize(size);
    generate(data.begin(), data.end(), [] { return static_cast<byte>(1); });

    size = 10;
    data.resize(size);
    generate(data.begin(), data.end(), [] { return static_cast<byte>(2); });

    cout << "vec: ";
    for (auto const b : stack) {
        cout << static_cast<int>(b);
    }
    cout << endl;
}

void raw()  {
    array<byte, 100> stack{};
    pmr::monotonic_buffer_resource res{stack.data(),
                                       stack.size() * sizeof(std::byte)};
    pmr::polymorphic_allocator<byte> alloc{&res};

    auto size = 90;
    auto* data = static_cast<byte*>(alloc.allocate(size));
    generate(data, data + size, [] { return static_cast<byte>(1); });

    size = 10;
    data = static_cast<byte*>(alloc.allocate(size));
    generate(data, data + size, [] { return static_cast<byte>(2); });

    cout << "raw: ";
    for (auto const b : stack) {
        cout << static_cast<int>(b);
    }
    cout << endl;
}

int main () {

    vec();
    raw();
}
