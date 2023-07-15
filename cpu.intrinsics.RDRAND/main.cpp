#include <cstdint>
#include <immintrin.h>
#include <iostream>

int main() {

    unsigned int rand;

    // Generate random value using RDRAND
    while (_rdrand32_step(&rand) == 0) {
    }

    std::cout << "Random number: " << rand << std::endl;

    // Generate 8 random bytes
    uint32_t rand_bytes[8];
    for (int i = 0; i < 8; i++) {
        _rdrand32_step(&rand_bytes[i]);
    }

    std::cout << "Random bytes: ";
    std::cout << std::hex << rand_bytes[0];
    for (int i = 1; i < 8; i++) {
        std::cout << "-" << std::hex << rand_bytes[i];
    }
    std::cout << std::endl;

    return 0;
}
