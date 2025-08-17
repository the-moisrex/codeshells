#include "webpp/unicode/unicode.hpp"
#include "webpp/unicode/normalization.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace webpp::unicode;

// Debug version of is_composable_to
template <webpp::stl::forward_iterator Iter, typename EIter = Iter, webpp::stl::forward_iterator CIter, typename CEIter = CIter>
    requires(webpp::stl::sentinel_for<EIter, Iter> && webpp::stl::sentinel_for<CEIter, CIter>)
[[nodiscard]] static constexpr bool
debug_is_composable_to(Iter spos, EIter const send, CIter cpos, CEIter const cend) noexcept {
    checked::utf32_forward_iter cp1_pin{spos, send};
    checked::utf32_forward_iter rep_cpin{cpos, cend};

    bool is_valid = true;
    std::cout << "=== Starting debug_is_composable_to ===\n";
    for (; !cp1_pin.at_end(); ++cp1_pin, ++rep_cpin) {
        std::cout << "Outer loop iteration\n";
        if (rep_cpin.at_end()) [[unlikely]] {
            std::cout << "rep_cpin.at_end() is true, returning false\n";
            return false;
        }
        auto       cp1         = *cp1_pin;
        auto const starter_ccp = *rep_cpin;
        std::cout << "cp1 (starter): U+" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(cp1) 
                  << ", starter_ccp: U+" << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(starter_ccp) << std::dec << "\n";
        
        auto       cp2_pin     = webpp::stl::next(cp1_pin);
        for (webpp::stl::int_fast16_t prev_ccc = -1; !cp2_pin.at_end(); ++cp2_pin) {
            auto const cp2         = *cp2_pin;
            auto const ccc         = static_cast<webpp::stl::int_fast16_t>(ccc_of(cp2));
            auto const replaced_cp = canonical_composed<max_utf32<char32_t>>(cp1, cp2);
            std::cout << "  Inner loop: cp2: U+" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(cp2) 
                      << ", ccc: " << std::dec << ccc
                      << ", replaced_cp: U+" << std::hex << std::setw(8) << std::setfill('0') << static_cast<unsigned int>(replaced_cp) << std::dec << "\n";
            
            if (prev_ccc < ccc && replaced_cp != max_utf32<char32_t>) {
                // found a composition
                std::cout << "  Found composition, updating cp1\n";
                cp1 = replaced_cp;
                continue;
            }
            if (ccc == 0) [[likely]] {
                std::cout << "  ccc == 0, breaking\n";
                break;
            }
            prev_ccc = ccc;
            ++rep_cpin;
            std::cout << "  Checking if *rep_cpin == cp2\n";
            is_valid &= !rep_cpin.at_end() && *rep_cpin == cp2;
        }
        std::cout << "Checking if starter_ccp == cp1\n";
        std::cout << "starter_ccp: U+" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(starter_ccp)
                  << ", cp1: U+" << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(cp1) << std::dec << "\n";
        is_valid &= starter_ccp == cp1;
    }
    std::cout << "Checking if rep_cpin.at_end()\n";
    is_valid &= rep_cpin.at_end();
    std::cout << "Final result: " << (is_valid ? "true" : "false") << "\n";
    std::cout << "=== Ending debug_is_composable_to ===\n";
    return is_valid;
}

int main() {
    // Simple test case: U+0041 + U+030A should compose to U+00C5
    std::u32string decomposed = {0x0041, 0x030A};
    std::u32string composed = {0x00C5};
    
    std::cout << "Simple test case:\n";
    std::cout << "Decomposed: ";
    for (char32_t cp : decomposed) {
        std::cout << "U+" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(cp) << " ";
    }
    std::cout << "\nComposed:   ";
    for (char32_t cp : composed) {
        std::cout << "U+" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(cp) << " ";
    }
    std::cout << "\n";
    
    auto result = canonical_composed(0x0041, 0x030A);
    std::cout << "Direct composition result: U+" << std::hex << std::setw(8) << std::setfill('0') << static_cast<unsigned int>(result) << std::dec << "\n";
    
    // Test our debug is_composable_to function
    bool is_composable = debug_is_composable_to(
        decomposed.begin(), decomposed.end(),
        composed.begin(), composed.end()
    );
    
    std::cout << "debug_is_composable_to result: " << (is_composable ? "true" : "false") << "\n";
    
    return 0;
}
