import standard;

#include <webpp/unicode/unicode.hpp>
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

// Testing UTF-16 code units:
// Unit 0: 0x212b
// Unit 1: 0x0303
// Unit 2: 0x0303
// Unit 3: 0x0303
// Unit 4: 0x0303
// Unit 5: 0x0303
// Unit 6: 0x033b
// Unit 7: 0x0303
// 
// Forward iteration:
// Code point 0: 0x0000212b
// Code point 1: 0x00000303
// Code point 2: 0x00000303
// Code point 3: 0x00000303
// Code point 4: 0x00000303
// Code point 5: 0x00000303
// Code point 6: 0x0000033b
// Code point 7: 0x00000303
// 
// Backward iteration:
// Code point 0: 0x00000303 (rit now at: 7)
// Code point 1: 0x0000033b (rit now at: 6)
// Code point 2: 0x00000303 (rit now at: 5)
// Code point 3: 0x00000303 (rit now at: 4)
// Code point 4: 0x00000303 (rit now at: 3)
// Code point 5: 0x00000303 (rit now at: 2)
// Code point 6: 0x00000303 (rit now at: 1)
// Code point 7: 0x0000212b (rit now at: 0)


