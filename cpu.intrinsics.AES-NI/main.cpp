#include <iostream>
#include <wmmintrin.h>
#include <cstdint>

int main() {

    // 16-byte aligned input, output, and key
    alignas(16)
        uint8_t input[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
                             0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
    alignas(16) uint8_t output[16];
    alignas(16)
        uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                           0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

    // Load input, output, and key into XMM registers
    __m128i xmm_input = _mm_load_si128((__m128i*)input);
    __m128i xmm_output = _mm_load_si128((__m128i*)output);
    __m128i xmm_key = _mm_load_si128((__m128i*)key);

    // Perform AES rounds
    xmm_output = _mm_xor_si128(xmm_input, xmm_key);
    for (int i = 0; i < 10; i++) {
        xmm_output = _mm_aesenc_si128(xmm_output, xmm_key);
    }

    // Store output
    _mm_store_si128((__m128i*)output, xmm_output);

    // Print result
    std::cout << "Input:     " << input << std::endl;
    std::cout << "Encrypted: ";
    for (int i = 0; i < 16; i++)
        std::cout << std::hex << (int)output[i];
    std::cout << std::endl;

    return 0;
}
