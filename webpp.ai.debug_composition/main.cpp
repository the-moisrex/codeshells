#include "webpp/unicode/normalization.hpp"
#include "webpp/unicode/unicode.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace webpp::unicode;

void print_string(const std::u32string& str, const char* name) {
    std::cout << name << ": ";
    for (char32_t cp : str) {
        std::cout << "U+" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(cp) << " ";
    }
    std::cout << std::dec << "\n";
}

int main() {
    // Test the specific case from the error
    // Src16: \x212B\x0303\x0303\x0303\x0303\x0303\x033B\x0303
    // NFC: \x00C5\x033B\x0303\x0303\x0303\x0303\x0303
    
    std::u32string input = {0x212B, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x033B, 0x0303};
    std::u32string expected_nfc = {0x00C5, 0x033B, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303};
    
    print_string(input, "Input");
    print_string(expected_nfc, "Expected NFC");
    
    std::cout << "isNFC(input): " << (isNFC(input.begin(), input.end()) ? "true" : "false") << "\n";
    std::cout << "isNFC(expected_nfc): " << (isNFC(expected_nfc.begin(), expected_nfc.end()) ? "true" : "false") << "\n";
    
    // Let's also check the quick check value
    auto qc_val = quick_check<norm_form::NFC>(expected_nfc.begin(), expected_nfc.end());
    std::cout << "Quick check value for expected NFC: ";
    switch (qc_val) {
        case quick_check_state::YES: std::cout << "YES\n"; break;
        case quick_check_state::NO: std::cout << "NO\n"; break;
        case quick_check_state::MAYBE: std::cout << "MAYBE\n"; break;
        default: std::cout << "UNKNOWN\n"; break;
    }
    
    return 0;
}
