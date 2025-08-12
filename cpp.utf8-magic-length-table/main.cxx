import standard;

#include <cstdint>
#include <print>

using namespace std;

int main() {
    std::array<uint8_t, 256> lens;
    lens.fill(1);

    for (uint8_t cp = 0; cp != 0xFF; ++cp) {
        lens[(cp & ~0b11) | 0b00] = 1;
        lens[(cp & ~0b11) | 0b01] = 1;
        // lens[(cp & ~0b11) | 0b10] = 1;
        lens[(cp & ~0b1111) | 0b1110] = 2;
        lens[(cp & ~0b11'1111) | 0b11'1010] = 3;
        lens[(cp & ~0b1111'1111) | 0b1110'1010] = 4;
    }

    for (uint8_t cp = 0; cp != 0xFF; ++cp) {
        print("{}, ", lens[cp]);
    }
    println();
    println("Tests: ");

    println("{} = 1", lens[0b01]);
    println("{}", lens[0b11]);
    println("{}", lens[0b1111]);
    println("{}", lens[0b1011]);
    println("{} = 2", lens[0b1110]);
    println("{} = 4", lens[0b1110'1010]);
    println("{}", lens[0b1010'1010]);

    return 0;
}
