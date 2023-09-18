#include <cassert>
#include <cstdint>
using namespace std;

// idea from: https://www.1024cores.net/home/lock-free-algorithms/tricks/pointer-packing
static constexpr size_t packed_ptr_size = 39;
static size_t lo_bits = 4;
static constexpr uintptr_t lo_mask = ((1ull << 4) - 1);
static constexpr uintptr_t hi_mask = ~((1ull << 43) - 1);

uintptr_t ptr_pack(void const volatile* p) {
    uintptr_t v = (uintptr_t)p;
    // assert((v & lo_mask) == 0);
    // assert((v & hi_mask) == 0);
    v = v >> lo_bits;
    return v;
}

void* ptr_unpack(uintptr_t v) {
    uintptr_t p = (v << lo_bits) & ~hi_mask;
    return (void*)p;
}

#include <iostream>
int main() {
    cout << "Size of uintptr_t: " << sizeof(uintptr_t) << endl;
    cout << "Size of void*: " << sizeof(void*) << endl;

    volatile int a = 10;
    volatile int* bptr = new int{10};
    volatile int* aptr = &a;
    cout << "B pointer: \t" << bptr << endl;
    cout << "B packed: \t" << ptr_pack(bptr) << endl;
    cout << "B unpacked: \t" << ptr_unpack(ptr_pack(bptr)) << endl;
    cout << "A pointer: \t" << aptr << endl;
    cout << "A packed: \t" << ptr_pack(aptr) << endl;
    cout << "A unpacked: \t" << ptr_unpack(ptr_pack(aptr)) << endl;
    delete bptr;
}
