#include "webpp/unicode/unicode.hpp"
#include <iostream>
#include <vector>
#include <iomanip>

using namespace webpp::unicode;

int main() {
    // Test data from the error:
    // Src16: \x212B\x0303\x0303\x0303\x0303\x0303\x033B\x0303
    // NFC: \x00C5\x033B\x0303\x0303\x0303\x0303\x0303
    
    std::vector<char16_t> test_data = {
        0x212B, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x033B, 0x0303
    };
    
    std::cout << "Testing UTF-16 code units:\n";
    for (size_t i = 0; i < test_data.size(); ++i) {
        std::cout << "Unit " << i << ": 0x" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(test_data[i]) << std::dec << "\n";
    }
    
    // Try to read code points forward
    std::cout << "\nForward iteration:\n";
    auto it = test_data.begin();
    auto end = test_data.end();
    size_t cp_index = 0;
    while (it != end) {
        auto cp = unchecked::next_code_point(it, end);
        std::cout << "Code point " << cp_index << ": 0x" << std::hex << std::setw(8) << std::setfill('0') << static_cast<unsigned int>(cp) << std::dec << "\n";
        cp_index++;
    }
    
    // Try to read code points backward
    std::cout << "\nBackward iteration:\n";
    auto rit = test_data.end();
    auto rbeg = test_data.begin();
    cp_index = 0;
    while (rit != rbeg) {
        auto cp = unchecked::prev_code_point(rit);
        std::cout << "Code point " << cp_index << ": 0x" << std::hex << std::setw(8) << std::setfill('0') << static_cast<unsigned int>(cp) << std::dec << " (rit now at: " << (rit - rbeg) << ")\n";
        cp_index++;
    }
    
    return 0;
}
