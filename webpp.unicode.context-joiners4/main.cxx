import standard;

#include <cstdint>
#include <cstddef>     // For size_t
#include <iterator>    // For iterator tags, distance, next, prev
#include <type_traits> // For iterator traits, is_base_of_v, is_same_v
#include <array>       // For lookup tables & examples
#include <print>       // C++23 for std::print

// --- Constants ---
constexpr char32_t ZWNJ = U'\u200C';
constexpr char32_t ZWJ = U'\u200D';
constexpr char32_t MAX_CODEPOINT = 0x10FFFF; // Maximum valid Unicode code point

/*
-------------------------------------------------------------------------------
 HOW TO REGENERATE UNICODE PROPERTY DATA USING AWK (Example Commands)
-------------------------------------------------------------------------------

 You need the Unicode Character Database (UCD) files. Download them from
 https://www.unicode.org/Public/UCD/latest/ucd/

 1. Extracting Virama Characters (Based on Canonical Combining Class = 9)
    File needed: UnicodeData.txt
    Format: CodePoint;Name;Category;CombiningClass;BidiClass;...

    Command:
    awk -F';' '$4 == "9" { printf "    0x%s, // %s\n", $1, $2 }' UnicodeData.txt > virama_list.txt

    Explanation:
    - `-F';'` sets the field separator to a semicolon.
    - `$4 == "9"` checks if the 4th field (Canonical Combining Class) is exactly "9".
    - `printf "    0x%s, // %s\n", $1, $2` formats the output as `0xXXXX, // Name`.
    - Redirect the output to a file (e.g., `virama_list.txt`).
    - Manually copy the generated list into the `virama_list` array below.

 2. Extracting Joining Types (R, L, D for Arabic Shaping)
    File needed: ArabicShaping.txt
    Format: CodePoint; Name; Joining Type; Joining Group

    Commands (generate separate lists):
    awk -F'; ' '/^[0-9A-F]/ && NF >= 3 && $3 == "R" { printf "    0x%s, // %s\n", $1, $2 }' ArabicShaping.txt > joining_r_list.txt
    awk -F'; ' '/^[0-9A-F]/ && NF >= 3 && $3 == "L" { printf "    0x%s, // %s\n", $1, $2 }' ArabicShaping.txt > joining_l_list.txt
    awk -F'; ' '/^[0-9A-F]/ && NF >= 3 && $3 == "D" { printf "    0x%s, // %s\n", $1, $2 }' ArabicShaping.txt > joining_d_list.txt

    Explanation:
    - `-F'; '` sets the field separator to semicolon followed by a space.
    - `/^[0-9A-F]/ && NF >= 3` skips comments/empty lines and ensures enough fields.
    - `$3 == "X"` checks if the 3rd field (Joining Type) matches R, L, or D.
    - `printf "    0x%s, // %s\n", $1, $2` formats the output.
    - Redirect output to separate files.
    - Manually copy the generated lists into the corresponding arrays below.

 3. Generating O(1) Lookup Tables (Offline Step)
    - The arrays below (`virama_list`, `joining_r_list`, etc.) are the *input* for this step.
    - Write a separate script (e.g., Python, Perl, or even C++) that reads these lists.
    - This script should generate C++ code containing `constexpr` arrays representing multi-level lookup tables or perfect hash functions.
    - The goal is to create functions like `generated_is_virama`, `generated_is_joining_r`, etc., that perform the lookup in O(1) time using the generated tables.
    - The simulation functions below (`detail::generated_...`) should be replaced by calls to these generated O(1) functions.

-------------------------------------------------------------------------------
*/


// --- O(1) Unicode Property Lookup (Simulated Precomputed Tables) ---

namespace unicode_lookup {
namespace detail {

// --- Data Arrays (Input for offline table generation) ---
constexpr char32_t virama_list[] = {
    0x094D,  0x09CD,  0x0A4D,  0x0ACD,  0x0B4D,  0x0BCD,  0x0C4D,  0x0CCD,
    0x0D3B,  0x0D3C,  0x0D4D,  0x0DCA,  0x0E3A,  0x0EBA,  0x0F84,  0x1039,
    0x103A,  0x1714,  0x1734,  0x17D2,  0x1A60,  0x1B44,  0x1BAA,  0x1BAB,
    0x1BF2,  0x1BF3,  0x2D7F,  0xA806,  0xA82C,  0xA8C4,  0xA953,  0xA9C0,
    0xAAF6,  0xABED,  0x10A3F, 0x11046, 0x1107F, 0x110B9, 0x11133, 0x11134,
    0x111C0, 0x11235, 0x112EA, 0x1134D, 0x11442, 0x114C2, 0x115BF, 0x1163F,
    0x116B6, 0x1172B, 0x11839, 0x1193D, 0x1193E, 0x119E0, 0x11A34, 0x11A47,
    0x11A99, 0x11C3F, 0x11D44, 0x11D45, 0x11D97};
constexpr char32_t joining_r_list[] = {
    0x622, 0x623, 0x624, 0x625, 0x627, 0x629, 0x62f, 0x630, 0x631,
    0x632, 0x648, 0x671, 0x672, 0x673, 0x675, 0x676, 0x677, 0x688,
    0x689, 0x68a, 0x68b, 0x68c, 0x68d, 0x68e, 0x68f, 0x690, 0x691,
    0x692, 0x693, 0x694, 0x695, 0x696, 0x697, 0x698, 0x699, 0x6c0,
    0x6c3, 0x6c4, 0x6c5, 0x6c6, 0x6c7, 0x6c8, 0x6c9, 0x6ca, 0x6cb,
    0x6cd, 0x6cf, 0x6d2, 0x6d3, 0x6d5, 0x6ee, 0x6ef, 0x710, 0x715,
    0x716, 0x717, 0x718, 0x719, 0x71e, 0x728, 0x72a, 0x72c, 0x72f,
    0x74d, 0x759, 0x75a, 0x75b, 0x854, 0x8aa, 0x8ab, 0x8ac};
constexpr char32_t joining_l_list[] = {0xa872};
constexpr char32_t joining_d_list[] = {
    0x620,  0x626,  0x628,  0x62a,  0x62b,  0x62c,  0x62d,  0x62e,  0x633,
    0x634,  0x635,  0x636,  0x637,  0x638,  0x639,  0x63a,  0x63b,  0x63c,
    0x63d,  0x63e,  0x63f,  0x641,  0x642,  0x643,  0x644,  0x645,  0x646,
    0x647,  0x649,  0x64a,  0x66e,  0x66f,  0x678,  0x679,  0x67a,  0x67b,
    0x67c,  0x67d,  0x67e,  0x67f,  0x680,  0x681,  0x682,  0x683,  0x684,
    0x685,  0x686,  0x687,  0x69a,  0x69b,  0x69c,  0x69d,  0x69e,  0x69f,
    0x6a0,  0x6a1,  0x6a2,  0x6a3,  0x6a4,  0x6a5,  0x6a6,  0x6a7,  0x6a8,
    0x6a9,  0x6aa,  0x6ab,  0x6ac,  0x6ad,  0x6ae,  0x6af,  0x6b0,  0x6b1,
    0x6b2,  0x6b3,  0x6b4,  0x6b5,  0x6b6,  0x6b7,  0x6b8,  0x6b9,  0x6ba,
    0x6bb,  0x6bc,  0x6bd,  0x6be,  0x6bf,  0x6c1,  0x6c2,  0x6cc,  0x6ce,
    0x6d0,  0x6d1,  0x6fa,  0x6fb,  0x6fc,  0x6ff,  0x712,  0x713,  0x714,
    0x71a,  0x71b,  0x71c,  0x71d,  0x71f,  0x720,  0x721,  0x722,  0x723,
    0x724,  0x725,  0x726,  0x727,  0x729,  0x72b,  0x72d,  0x72e,  0x74e,
    0x74f,  0x750,  0x751,  0x752,  0x753,  0x754,  0x755,  0x756,  0x757,
    0x758,  0x75c,  0x75d,  0x75e,  0x75f,  0x760,  0x761,  0x762,  0x763,
    0x764,  0x765,  0x766,  0x850,  0x851,  0x852,  0x853,  0x855,  0x8a0,
    0x8a2,  0x8a3,  0x8a4,  0x8a5,  0x8a6,  0x8a7,  0x8a8,  0x8a9,  0x1807,
    0x1820, 0x1821, 0x1822, 0x1823, 0x1824, 0x1825, 0x1826, 0x1827, 0x1828,
    0x1829, 0x182a, 0x182b, 0x182c, 0x182d, 0x182e, 0x182f, 0x1830, 0x1831,
    0x1832, 0x1833, 0x1834, 0x1835, 0x1836, 0x1837, 0x1838, 0x1839, 0x183a,
    0x183b, 0x183c, 0x183d, 0x183e, 0x183f, 0x1840, 0x1841, 0x1842, 0x1843,
    0x1844, 0x1845, 0x1846, 0x1847, 0x1848, 0x1849, 0x184a, 0x184b, 0x184c,
    0x184d, 0x184e, 0x184f, 0x1850, 0x1851, 0x1852, 0x1853, 0x1854, 0x1855,
    0x1856, 0x1857, 0x1858, 0x1859, 0x185a, 0x185b, 0x185c, 0x185d, 0x185e,
    0x185f, 0x1860, 0x1861, 0x1862, 0x1863, 0x1864, 0x1865, 0x1866, 0x1867,
    0x1868, 0x1869, 0x186a, 0x186b, 0x186c, 0x186d, 0x186e, 0x186f, 0x1870,
    0x1871, 0x1872, 0x1873, 0x1874, 0x1875, 0x1876, 0x1877, 0x1887, 0x1888,
    0x1889, 0x188a, 0x188b, 0x188c, 0x188d, 0x188e, 0x188f, 0x1890, 0x1891,
    0x1892, 0x1893, 0x1894, 0x1895, 0x1896, 0x1897, 0x1898, 0x1899, 0x189a,
    0x189b, 0x189c, 0x189d, 0x189e, 0x189f, 0x18a0, 0x18a1, 0x18a2, 0x18a3,
    0x18a4, 0x18a5, 0x18a6, 0x18a7, 0x18a8, 0x18aa, 0xa840, 0xa841, 0xa842,
    0xa843, 0xa844, 0xa845, 0xa846, 0xa847, 0xa848, 0xa849, 0xa84a, 0xa84b,
    0xa84c, 0xa84d, 0xa84e, 0xa84f, 0xa850, 0xa851, 0xa852, 0xa853, 0xa854,
    0xa855, 0xa856, 0xa857, 0xa858, 0xa859, 0xa85a, 0xa85b, 0xa85c, 0xa85d,
    0xa85e, 0xa85f, 0xa860, 0xa861, 0xa862, 0xa863, 0xa864, 0xa865, 0xa866,
    0xa867, 0xa868, 0xa869, 0xa86a, 0xa86b, 0xa86c, 0xa86d, 0xa86e, 0xa86f,
    0xa870, 0xa871};


// --- Multi-level Table Simulation (Replace with generated O(1) functions) ---
// These functions SIMULATE the O(1) lookup using linear search on the input lists.
// This is INEFFICIENT and only for making the example runnable.
// Replace the BODY of these functions with calls to your actual generated O(1) lookup code.

constexpr bool generated_is_virama(char32_t cp) noexcept {
    for (char32_t v : virama_list) { if (cp == v) return true; }
    return false;
}

constexpr bool generated_is_joining_r(char32_t cp) noexcept {
    for (char32_t r : joining_r_list) { if (cp == r) return true; }
    return false;
}

constexpr bool generated_is_joining_l(char32_t cp) noexcept {
    for (char32_t l : joining_l_list) { if (cp == l) return true; }
    return false;
}

constexpr bool generated_is_joining_d(char32_t cp) noexcept {
    for (char32_t d : joining_d_list) { if (cp == d) return true; }
    return false;
}

} // namespace detail

// --- Public O(1) Lookup Functions ---

/**
 * @brief Checks if a codepoint is a Virama character using precomputed tables. O(1).
 * @param cp The char32_t code point to check.
 * @return true if the code point is a Virama, false otherwise.
 * @noexcept
 */
constexpr bool is_virama(char32_t cp) noexcept {
    // Replace with actual generated O(1) lookup
    return detail::generated_is_virama(cp);
}

/**
 * @brief Checks if a codepoint has Joining Type L or D using precomputed tables. O(1).
 * @param cp The char32_t code point to check.
 * @return true if Joining Type is L or D, false otherwise.
 * @noexcept
 */
constexpr bool is_joining_l_or_d(char32_t cp) noexcept {
    // Replace with actual generated O(1) lookup (might combine L and D tables)
    return detail::generated_is_joining_l(cp) || detail::generated_is_joining_d(cp);
}

/**
 * @brief Checks if a codepoint has Joining Type R or D using precomputed tables. O(1).
 * @param cp The char32_t code point to check.
 * @return true if Joining Type is R or D, false otherwise.
 * @noexcept
 */
constexpr bool is_joining_r_or_d(char32_t cp) noexcept {
    // Replace with actual generated O(1) lookup (might combine R and D tables)
    return detail::generated_is_joining_r(cp) || detail::generated_is_joining_d(cp);
}

} // namespace unicode_lookup


// --- Core Validation Logic (Single Pass, Inlined, O(1) Lookup, User Logic) ---

namespace validation {

/**
 * @brief Validates a label according to RFC 5892 context rules A.2 (ZWJ) and a
 *        modified A.1 (ZWNJ) rule matching the user's provided logic, using O(1) lookups.
 *
 * Iterates through the label defined by the iterator pair [begin, end) in a single pass.
 * Checks if every occurrence of U+200D (ZWJ) satisfies the Virama rule.
 * Checks if every occurrence of U+200C (ZWNJ) satisfies EITHER the Virama rule OR
 *        finds *any* L/D character before it AND *any* R/D character after it
 *        (without skipping any characters).
 * Uses precomputed lookup tables (simulated) for O(1) character property checks.
 * Logic is inlined into the main loop, optimizing for the happy path.
 *
 * Requires at least Bidirectional Iterators for the ZWNJ backward search.
 *
 * @tparam BidiIter A Bidirectional Iterator type whose value_type is char32_t.
 * @param begin Iterator to the beginning of the label.
 * @param end Iterator to the end of the label.
 * @return true If all ZWJ and ZWNJ characters in the label satisfy their respective context rules, false otherwise.
 * @noexcept Guarantees not to throw exceptions.
 */
template<typename BidiIter>
constexpr bool validate_rfc5892_context_user_logic_o1(BidiIter begin, BidiIter end) noexcept {
    static_assert(std::is_base_of_v<std::bidirectional_iterator_tag,
                  typename std::iterator_traits<BidiIter>::iterator_category>,
                  "validate_rfc5892_context_user_logic_o1 requires at least bidirectional iterators");
    static_assert(std::is_same_v<typename std::iterator_traits<BidiIter>::value_type, char32_t>,
                  "Iterator value_type must be char32_t");

    char32_t prev_cp = 0; // Immediate preceding code point (0 if none)

    for (BidiIter it = begin; it != end; ++it) {
        const char32_t current_cp = *it;

        // --- Happy Path Optimization ---
        // If not ZWNJ or ZWJ, just update context and continue.
        if (current_cp != ZWNJ && current_cp != ZWJ) {
            prev_cp = current_cp;
            continue; // Skip ZWJ/ZWNJ checks
        }

        // --- ZWJ Check (Rule A.2) ---
        if (current_cp == ZWJ) {
            // Rule: Must be preceded by a Virama.
            // Check if prev_cp is valid (not beginning) AND is a Virama.
            if (!(prev_cp != 0 && unicode_lookup::is_virama(prev_cp))) {
                return false; // Invalid ZWJ context
            }
            // If valid, fall through to update context at the end.
        }
        // --- ZWNJ Check (Modified Rule A.1 - based on user code) ---
        else if (current_cp == ZWNJ) {
            bool rule_met = false;

            // Check 1: Preceded by Virama?
            if (prev_cp != 0 && unicode_lookup::is_virama(prev_cp)) {
                rule_met = true;
            }

            // Check 2: Find L/D before AND R/D after (only if Virama rule didn't apply)
            if (!rule_met) {
                bool found_l_or_d_before = false;
                // Search backwards from the character *before* the ZWNJ
                if (it != begin) { // Check if we are not already at the beginning
                    BidiIter before_it = std::prev(it);
                    while (true) {
                        if (unicode_lookup::is_joining_l_or_d(*before_it)) {
                            found_l_or_d_before = true;
                            break;
                        }
                        if (before_it == begin) break; // Reached beginning
                        --before_it;
                    }
                }

                // Only proceed if L/D was found before
                if (found_l_or_d_before) {
                    bool found_r_or_d_after = false;
                    // Search forwards from the character *after* the ZWNJ
                    BidiIter after_it = std::next(it);
                    while (after_it != end) {
                        if (unicode_lookup::is_joining_r_or_d(*after_it)) {
                            found_r_or_d_after = true;
                            break;
                        }
                        ++after_it;
                    }

                    // If both conditions are met
                    if (found_r_or_d_after) {
                        rule_met = true;
                    }
                }
                // If L/D wasn't found before, or R/D wasn't found after, rule_met remains false.
            }

            // If neither the Virama rule nor the modified Regex rule was met
            if (!rule_met) {
                return false; // Invalid ZWNJ context
            }
            // If valid, fall through to update context at the end.
        }

        // --- Update Context ---
        prev_cp = current_cp;
    }

    // If the loop completes without finding any invalid context
    return true;
}

} // namespace validation


// --- Example Usage ---
#include <vector>   // For easy example label creation
#include <string>   // For easy example label creation (using u32string)
#include <format>   // For formatting output with std::print

// Helper to print label for clarity
template<typename Iter>
void print_label(Iter begin, Iter end) {
    for (Iter it = begin; it != end; ++it) {
        std::print("U+{:04X} ", static_cast<uint32_t>(*it));
    }
    std::println(""); // Newline
}

int main() {
    std::println("RFC 5892 Contextual Rule Checker (A.2 ZWJ, Modified A.1 ZWNJ)");
    std::println("Inlined single pass, O(1) lookup (simulated), User Logic, std::print");
    std::println("--- IMPORTANT: Uses SIMULATED O(1) Unicode properties! ---");
    std::println("");

    // Use the validation function implementing the user's logic
    using validation::validate_rfc5892_context_user_logic_o1;

    // Example 1: ZWJ after Virama (Valid)
    constexpr std::array<char32_t, 4> label1_arr = {U'\u0915', U'\u094D', ZWJ, U'\u092F'}; // क + ् + ZWJ + य
    constexpr bool result1 = validate_rfc5892_context_user_logic_o1(label1_arr.begin(), label1_arr.end());
    static_assert(result1 == true, "Test Case 1 Failed");
    std::print("Label 1: "); print_label(label1_arr.begin(), label1_arr.end());
    std::println("Validation Result: {}", (result1 ? "Valid" : "Invalid"));
    std::println("");

    // Example 2: ZWNJ after Virama (Valid)
    constexpr std::array<char32_t, 4> label2_arr = {U'\u0915', U'\u094D', ZWNJ, U'\u092F'}; // क + ् + ZWNJ + य
    constexpr bool result2 = validate_rfc5892_context_user_logic_o1(label2_arr.begin(), label2_arr.end());
    static_assert(result2 == true, "Test Case 2 Failed");
    std::print("Label 2: "); print_label(label2_arr.begin(), label2_arr.end());
    std::println("Validation Result: {}", (result2 ? "Valid" : "Invalid"));
    std::println("");

    // Example 3: ZWJ not after Virama (Invalid)
    constexpr std::array<char32_t, 3> label3_arr = {U'\u0915', ZWJ, U'\u092F'}; // क + ZWJ + य
    constexpr bool result3 = validate_rfc5892_context_user_logic_o1(label3_arr.begin(), label3_arr.end());
    static_assert(result3 == false, "Test Case 3 Failed");
    std::print("Label 3: "); print_label(label3_arr.begin(), label3_arr.end());
    std::println("Validation Result: {}", (result3 ? "Valid" : "Invalid"));
    std::println("");

    // Example 4: ZWNJ context (User Logic: Find L/D before, R/D after) -> Valid
    // U+0644 (LAM, D) before, U+0627 (ALEF, R) after.
    constexpr std::array<char32_t, 3> label4_arr = {U'\u0644', ZWNJ, U'\u0627'}; // ل + ZWNJ + ا
    constexpr bool result4 = validate_rfc5892_context_user_logic_o1(label4_arr.begin(), label4_arr.end());
    static_assert(result4 == true, "Test Case 4 Failed");
    std::print("Label 4: "); print_label(label4_arr.begin(), label4_arr.end());
    std::println("Validation Result: {}", (result4 ? "Valid" : "Invalid"));
    std::println("");

    // Example 5: ZWNJ context (User Logic: Find L/D before, R/D after) -> Valid
    // U+0627 (ALEF, R) before (fails L/D check), U+0644 (LAM, D) after (is R/D).
    // Since the L/D check before fails, the overall result is false.
    constexpr std::array<char32_t, 3> label5_arr = {U'\u0627', ZWNJ, U'\u0644'}; // ا + ZWNJ + ل
    constexpr bool result5 = validate_rfc5892_context_user_logic_o1(label5_arr.begin(), label5_arr.end());
    static_assert(result5 == false, "Test Case 5 Failed");
    std::print("Label 5: "); print_label(label5_arr.begin(), label5_arr.end());
    std::println("Validation Result: {}", (result5 ? "Valid" : "Invalid"));
    std::println("");

     // Example 6: ZWNJ with other characters (User Logic: Find L/D before, R/D after) -> Valid
     // U+0644 (LAM, D) is found before. U+0627 (ALEF, R) is found after.
     // The intervening U+064B, U+061C don't affect the find operation.
    constexpr std::array<char32_t, 5> label6_arr = {U'\u0644', U'\u064B', ZWNJ, U'\u061C', U'\u0627'}; // ل +  ً + ZWNJ +  ؜ + ا
    constexpr bool result6 = validate_rfc5892_context_user_logic_o1(label6_arr.begin(), label6_arr.end());
    static_assert(result6 == true, "Test Case 6 Failed");
    std::print("Label 6: "); print_label(label6_arr.begin(), label6_arr.end());
    std::println("Validation Result: {}", (result6 ? "Valid" : "Invalid"));
    std::println("");


    // Example 7: ZWNJ at beginning (Invalid - Virama fails, L/D search before fails)
    constexpr std::array<char32_t, 2> label7_arr = {ZWNJ, U'\u0644'};
    constexpr bool result7 = validate_rfc5892_context_user_logic_o1(label7_arr.begin(), label7_arr.end());
    static_assert(result7 == false, "Test Case 7 Failed");
    std::print("Label 7: "); print_label(label7_arr.begin(), label7_arr.end());
    std::println("Validation Result: {}", (result7 ? "Valid" : "Invalid"));
    std::println("");

    // Example 8: Label with no ZWJ/ZWNJ (Valid by default)
    constexpr std::array<char32_t, 3> label8_arr = {U'a', U'b', U'c'};
    constexpr bool result8 = validate_rfc5892_context_user_logic_o1(label8_arr.begin(), label8_arr.end());
    static_assert(result8 == true, "Test Case 8 Failed");
    std::print("Label 8: "); print_label(label8_arr.begin(), label8_arr.end());
    std::println("Validation Result: {}", (result8 ? "Valid" : "Invalid"));
    std::println("");

    // Example 10: ZWNJ at end (Invalid - Virama fails, R/D search after fails)
    constexpr std::array<char32_t, 2> label10_arr = {U'\u0644', ZWNJ}; // LAM (D) + ZWNJ
    constexpr bool result10 = validate_rfc5892_context_user_logic_o1(label10_arr.begin(), label10_arr.end());
    static_assert(result10 == false, "Test Case 10 Failed");
    std::print("Label 10: "); print_label(label10_arr.begin(), label10_arr.end());
    std::println("Validation Result: {}", (result10 ? "Valid" : "Invalid"));
    std::println("");

    // Example 11: ZWNJ context (User Logic) - More complex
    // L D T ZWNJ T R D -> Valid (Finds L/D before, finds R/D after)
    constexpr std::array<char32_t, 7> label11_arr = {
        0xa872, // L
        0x0644, // D
        0x064B, // T (ignored by user logic search)
        ZWNJ,
        0x061C, // T (ignored by user logic search)
        0x0627, // R
        0x0628  // D
    };
    constexpr bool result11 = validate_rfc5892_context_user_logic_o1(label11_arr.begin(), label11_arr.end());
    static_assert(result11 == true, "Test Case 11 Failed");
    std::print("Label 11: "); print_label(label11_arr.begin(), label11_arr.end());
    std::println("Validation Result: {}", (result11 ? "Valid" : "Invalid"));
    std::println("");

    // Example 12: ZWNJ context (User Logic) - Failing case
    // R T ZWNJ T L -> Invalid (Finds R before -> fails L/D check)
     constexpr std::array<char32_t, 5> label12_arr = {
        0x0627, // R
        0x064B, // T (ignored)
        ZWNJ,
        0x061C, // T (ignored)
        0xa872  // L
    };
    constexpr bool result12 = validate_rfc5892_context_user_logic_o1(label12_arr.begin(), label12_arr.end());
    static_assert(result12 == false, "Test Case 12 Failed");
    std::print("Label 12: "); print_label(label12_arr.begin(), label12_arr.end());
    std::println("Validation Result: {}", (result12 ? "Valid" : "Invalid"));
    std::println("");


    return 0;
}
