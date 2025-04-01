import standard;

#include <cstddef> // For size_t
#include <cstdint>
#include <iterator>    // For iterator tags, distance
#include <type_traits> // For iterator traits, is_base_of_v

// --- Constants ---
constexpr char32_t ZWNJ = U'\u200C';
constexpr char32_t ZWJ = U'\u200D';
// Note: Canonical Combining Class for Virama is 9, but we check specific code
// points below.

/*
-------------------------------------------------------------------------------
 HOW TO EXTRACT UNICODE DATA USING AWK (Example Commands)
-------------------------------------------------------------------------------

 You need the Unicode Character Database (UCD) files. Download them from
 https://www.unicode.org/Public/UCD/latest/ucd/

 1. Extracting Virama Characters (Based on Canonical Combining Class = 9)
    File needed: UnicodeData.txt
    Format: CodePoint;Name;Category;CombiningClass;BidiClass;...

    Command:
    awk -F';' '$4 == "9" { printf "           cp == U'\''\\u%s'\'' || // %s\n",
$1, $2 }' UnicodeData.txt

    Explanation:
    - `-F';'` sets the field separator to a semicolon.
    - `$4 == "9"` checks if the 4th field (Canonical Combining Class) is exactly
"9".
    - `printf "           cp == U'\''\\u%s'\'' || // %s\n", $1, $2` formats the
output:
        - `cp == U'\uXXXX'` creates the C++ comparison code using the 1st field
($1, Code Point).
        - `|| // %s` adds the logical OR and comments the character name (2nd
field, $2).
    - Pipe the output of this command into the `is_virama` function body.
Remember to remove the last `||`.

 2. Extracting Joining Types (for Arabic Shaping)
    File needed: ArabicShaping.txt
    Format: CodePoint; Name; Joining Type; Joining Group

    Command (using gawk for trim):
    gawk -F'; ' '/^[0-9A-F]/ && NF >= 3 { cp=$1; type=trim($3); name=trim($2); \
        jt="Other"; \
        if(type=="R") jt="Right_Joining"; \
        else if(type=="L") jt="Left_Joining"; \
        else if(type=="D") jt="Dual_Joining"; \
        else if(type=="T") jt="Transparent"; \
        if(jt != "Other") printf "    if (cp == U'\''\\u%s'\'') return
JoiningType::%s; // %s\n", cp, jt, name; \
    }' ArabicShaping.txt

    Explanation:
    - `-F'; '` sets the field separator to semicolon followed by a space.
    - `/^[0-9A-F]/ && NF >= 3` skips comments/empty lines and ensures enough
fields.
    - `cp=$1; type=trim($3); name=trim($2);` assigns fields to variables,
trimming whitespace (needs `gawk` or similar).
    - `jt="Other"; if(type=="R")...` maps the single-letter type from the file
(R, L, D, T) to the corresponding enum member name.
    - `if(jt != "Other") printf ...` prints the C++ `if` statement for relevant
types.
    - Pipe the output into the `get_joining_type` function body, likely before
the default `return JoiningType::Other;`. Adjust enum names in the script if
they differ.
    - Note: Joining Types 'C' (Join_Causing) and 'U' (Non_Joining) are mapped to
`Other` here as they aren't explicitly used in the ZWNJ regex rule A.1.

-------------------------------------------------------------------------------
*/

// --- Minimalist Constexpr Unicode Property Helpers (STUBS) ---

/**
 * @brief Checks if a codepoint is a Virama character.
 * @param cp The char32_t code point to check.
 * @return true if the code point is a Virama, false otherwise.
 * @noexcept
 *
 * Data derived from UnicodeData.txt where Canonical_Combining_Class=9.
 * Use the awk script above to generate this list from the UCD.
 */
constexpr bool is_virama(char32_t cp) noexcept {
    // Add generated code points here, for example:
    return cp == U'\u094D' ||    // DEVANAGARI SIGN VIRAMA
           cp == U'\u09CD' ||    // BENGALI SIGN VIRAMA
           cp == U'\u0A4D' ||    // GURMUKHI SIGN VIRAMA
           cp == U'\u0ACD' ||    // GUJARATI SIGN VIRAMA
           cp == U'\u0B4D' ||    // ORIYA SIGN VIRAMA
           cp == U'\u0BCD' ||    // TAMIL SIGN VIRAMA
           cp == U'\u0C4D' ||    // TELUGU SIGN VIRAMA
           cp == U'\u0CCD' ||    // KANNADA SIGN VIRAMA
           cp == U'\u0D4D' ||    // MALAYALAM SIGN VIRAMA
           cp == U'\u0DCA' ||    // SINHALA SIGN AL-LAKUNA
           cp == U'\u1039' ||    // MYANMAR SIGN VIRAMA
           cp == U'\u17D2' ||    // KHMER SIGN COENG
           cp == U'\u1B44' ||    // BALINESE ADEG ADEG
           cp == U'\u1BAA' ||    // SUNDANESE SIGN PAMAAEH
           cp == U'\u1BAB' ||    // SUNDANESE SIGN VIRAMA
           cp == U'\uA953' ||    // REJANG VIRAMA
           cp == U'\uAAF6' ||    // TAI VIET SIGN SAKOT
           cp == U'\u{1104D}' || // BRAHMI SIGN VIRAMA
           cp == U'\u{111C0}' || // SHARADA SIGN VIRAMA
           cp == U'\u{11A3A}' || // ZANABAZAR SQUARE SIGN VIRAMA
           cp == U'\u{11A99}' || // SOYOMBO SIGN VIRAMA
           // Add more as needed from UCD...
           false; // Base case if none match
}

// Minimal Joining Type enum needed for Rule A.1
enum class JoiningType {
    Right_Joining, // R
    Left_Joining,  // L
    Dual_Joining,  // D
    Transparent,   // T
    Other          // Represents U, C, or anything not relevant/known
};

/**
 * @brief Gets the JoiningType for Rule A.1 context check. VERY LIMITED STUB.
 * @param cp The char32_t code point to check.
 * @return The JoiningType enum value.
 * @noexcept
 *
 * Data derived from ArabicShaping.txt.
 * Use the awk script above to generate the checks from the UCD.
 */
constexpr JoiningType get_joining_type(char32_t cp) noexcept {
    // Special case ZWNJ/ZWJ themselves are often treated as Transparent in
    // context checks
    if (cp == ZWNJ || cp == ZWJ)
        return JoiningType::Transparent;

    // Add generated checks here, for example:
    if (cp == U'\u064B')
        return JoiningType::Transparent; // ARABIC FATHATAN
    if (cp == U'\u061C')
        return JoiningType::Transparent; // ARABIC LETTER MARK
    // ... many more Transparent marks (Mn, Me, Cf categories often are)

    if (cp == U'\u0628')
        return JoiningType::Dual_Joining; // ARABIC LETTER BEH
    if (cp == U'\u0644')
        return JoiningType::Dual_Joining; // ARABIC LETTER LAM
    // ... many more Dual_Joining

    if (cp == U'\u0627')
        return JoiningType::Right_Joining; // ARABIC LETTER ALEF
    // ... many more Right_Joining

    // if (cp == U'\uXXXX') return JoiningType::Left_Joining; // Example
    // Left_Joining

    // Default for anything else in this limited stub
    return JoiningType::Other;
}

// --- Core Validation Logic ---

namespace detail {

    // Helper struct to hold context information gathered during the loop
    struct ContextInfo {
        char32_t prev_cp = 0; // The immediately preceding code point, 0 if none
        bool prev_cp_is_valid =
            false; // Flag indicating if prev_cp holds a valid char

        // Information about the nearest preceding non-transparent character
        char32_t prev_non_transparent_cp = 0;
        JoiningType prev_non_transparent_jt = JoiningType::Other;
        bool prev_non_transparent_is_valid = false;
    };

    // Updates the context info based on the current character
    constexpr void update_context(ContextInfo& context,
                                  char32_t current_cp) noexcept {
        context.prev_cp = current_cp;
        context.prev_cp_is_valid = true;

        JoiningType current_jt = get_joining_type(current_cp);
        if (current_jt != JoiningType::Transparent) {
            context.prev_non_transparent_cp = current_cp;
            context.prev_non_transparent_jt = current_jt;
            context.prev_non_transparent_is_valid = true;
        }
        // If current IS transparent, the previous non-transparent info remains
        // unchanged
    }

    // Finds the joining type of the first non-transparent character *after*
    // current_iter
    template <typename FwdIter>
    constexpr JoiningType find_following_joining_type(FwdIter current_iter,
                                                      FwdIter end) noexcept {
        static_assert(
            std::is_base_of_v<
                std::forward_iterator_tag,
                typename std::iterator_traits<FwdIter>::iterator_category>,
            "find_following_joining_type requires at least forward iterators");

        FwdIter it = current_iter;
        // Advance past the current character (e.g., the ZWNJ itself)
        if (it != end) {
            ++it;
        }

        while (it != end) {
            char32_t cp = *it;
            JoiningType jt = get_joining_type(cp);
            if (jt != JoiningType::Transparent) {
                return jt; // Found the first non-transparent
            }
            ++it;
        }
        return JoiningType::Other; // Reached end without finding one
    }

    // Checks Rule A.1 (ZWNJ) context using gathered context and lookahead
    template <typename FwdIter>
    constexpr bool check_rule_a1_zwnj_context(
        const ContextInfo& context,
        FwdIter current_iter, // Iterator pointing to the ZWNJ
        FwdIter end) noexcept {
        // Check 1: If Canonical_Combining_Class(Before(cp)) == Virama
        // Use the immediately preceding character from context
        if (context.prev_cp_is_valid && is_virama(context.prev_cp)) {
            return true; // Rule met
        }

        // Check 2: RegExpMatch((Join_Type:{L,D})(Join_Type:T)* ZWNJ
        // (Join_Type:T)*(Join_Type:{R,D})) Use the preceding non-transparent
        // info from context
        if (context.prev_non_transparent_is_valid &&
            (context.prev_non_transparent_jt == JoiningType::Left_Joining ||
             context.prev_non_transparent_jt == JoiningType::Dual_Joining)) {
            // Now perform lookahead for the following non-transparent character
            JoiningType following_jt =
                find_following_joining_type(current_iter, end);

            // Check if following type is R or D
            if (following_jt == JoiningType::Right_Joining ||
                following_jt == JoiningType::Dual_Joining) {
                return true; // Regex rule met
            }
        }

        // If neither condition was met
        return false;
    }

    // Checks Rule A.2 (ZWJ) context using gathered context
    constexpr bool
    check_rule_a2_zwj_context(const ContextInfo& context) noexcept {
        // Check: If Canonical_Combining_Class(Before(cp)) == Virama
        // Use the immediately preceding character from context
        return context.prev_cp_is_valid && is_virama(context.prev_cp);
    }

} // namespace detail

/**
 * @brief Validates a label according to RFC 5892 context rules A.1 (ZWNJ) and
 * A.2 (ZWJ).
 *
 * Iterates through the label defined by the iterator pair [begin, end) in a
 * single pass. Checks if every occurrence of U+200C (ZWNJ) and U+200D (ZWJ)
 * satisfies its respective contextual validity rules (Lookup=True rules only).
 *
 * Requires at least Forward Iterators. Lookahead is performed when necessary
 * for ZWNJ.
 *
 * @tparam FwdIter A Forward Iterator type.
 * @param begin Iterator to the beginning of the label (sequence of char32_t).
 * @param end Iterator to the end of the label.
 * @return true If all ZWNJ and ZWJ characters in the label satisfy their
 * context rules, false otherwise.
 * @noexcept Guarantees not to throw exceptions.
 */
template <typename FwdIter>
constexpr bool validate_rfc5892_context_single_pass(FwdIter begin,
                                                    FwdIter end) noexcept {
    static_assert(
        std::is_base_of_v<
            std::forward_iterator_tag,
            typename std::iterator_traits<FwdIter>::iterator_category>,
        "validate_rfc5892_context_single_pass requires at least forward "
        "iterators");

    detail::ContextInfo context; // Stores context from previous characters

    for (FwdIter it = begin; it != end; ++it) {
        const char32_t current_cp = *it;

        // --- Check rules based on the *current* character and *past* context
        // ---
        if (current_cp == ZWNJ) {
            // Apply Rule A.1 check using past context and lookahead from
            // current position
            if (!detail::check_rule_a1_zwnj_context(context, it, end)) {
                return false; // Context rule failed for this ZWNJ
            }
        } else if (current_cp == ZWJ) {
            // Apply Rule A.2 check using only past context
            if (!detail::check_rule_a2_zwj_context(context)) {
                return false; // Context rule failed for this ZWJ
            }
        }
        // No context rules to check for other characters in this function

        // --- Update context *after* checking the current character ---
        detail::update_context(context, current_cp);
    }

    // If the loop completes without finding any invalid context
    return true;
}

// --- Example Usage ---
#include <array>    // For constexpr examples
#include <iostream> // For printing results
#include <string>   // For easy example label creation (using u32string)
#include <vector>   // For easy example label creation

// Helper to print label for clarity
template <typename Iter>
void print_label(Iter begin, Iter end) {
    for (Iter it = begin; it != end; ++it) {
        std::cout << "U+" << std::hex << static_cast<uint32_t>(*it) << std::dec
                  << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "RFC 5892 Contextual Rule Checker (A.1 ZWNJ, A.2 ZWJ - "
                 "Lookup=True)\n";
    std::cout << "Iterator-based, single pass, noexcept, char32_t\n";
    std::cout << "--- IMPORTANT: Uses STUBBED Unicode properties! ---"
              << std::endl
              << std::endl;

    // Example 1: ZWJ after Virama (Valid)
    constexpr std::array<char32_t, 4> label1_arr = {
        U'\u0915', U'\u094D', ZWJ, U'\u092F'}; // क + ् + ZWJ + य
    constexpr bool result1 = validate_rfc5892_context_single_pass(
        label1_arr.begin(), label1_arr.end());
    static_assert(result1 == true, "Test Case 1 Failed");
    std::cout << "Label 1: ";
    print_label(label1_arr.begin(), label1_arr.end());
    std::cout << "Validation Result: " << (result1 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 2: ZWNJ after Virama (Valid)
    constexpr std::array<char32_t, 4> label2_arr = {
        U'\u0915', U'\u094D', ZWNJ, U'\u092F'}; // क + ् + ZWNJ + य
    constexpr bool result2 = validate_rfc5892_context_single_pass(
        label2_arr.begin(), label2_arr.end());
    static_assert(result2 == true, "Test Case 2 Failed");
    std::cout << "Label 2: ";
    print_label(label2_arr.begin(), label2_arr.end());
    std::cout << "Validation Result: " << (result2 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 3: ZWJ not after Virama (Invalid)
    constexpr std::array<char32_t, 3> label3_arr = {U'\u0915', ZWJ,
                                                    U'\u092F'}; // क + ZWJ + य
    constexpr bool result3 = validate_rfc5892_context_single_pass(
        label3_arr.begin(), label3_arr.end());
    static_assert(result3 == false, "Test Case 3 Failed");
    std::cout << "Label 3: ";
    print_label(label3_arr.begin(), label3_arr.end());
    std::cout << "Validation Result: " << (result3 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 4: ZWNJ in Arabic context (L/D + ZWNJ + R/D) -> Valid
    constexpr std::array<char32_t, 3> label4_arr = {U'\u0644', ZWNJ,
                                                    U'\u0627'}; // ل + ZWNJ + ا
    constexpr bool result4 = validate_rfc5892_context_single_pass(
        label4_arr.begin(), label4_arr.end());
    static_assert(result4 == true, "Test Case 4 Failed");
    std::cout << "Label 4: ";
    print_label(label4_arr.begin(), label4_arr.end());
    std::cout << "Validation Result: " << (result4 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 5: ZWNJ invalid Arabic context (R + ZWNJ + D) -> Invalid
    constexpr std::array<char32_t, 3> label5_arr = {U'\u0627', ZWNJ,
                                                    U'\u0644'}; // ا + ZWNJ + ل
    constexpr bool result5 = validate_rfc5892_context_single_pass(
        label5_arr.begin(), label5_arr.end());
    static_assert(result5 == false, "Test Case 5 Failed");
    std::cout << "Label 5: ";
    print_label(label5_arr.begin(), label5_arr.end());
    std::cout << "Validation Result: " << (result5 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 6: ZWNJ with Transparent characters (L/D + T* + ZWNJ + T* + R/D)
    // -> Valid
    constexpr std::array<char32_t, 5> label6_arr = {
        U'\u0644', U'\u064B', ZWNJ, U'\u061C',
        U'\u0627'}; // ل +  ً + ZWNJ +  ؜ + ا
    constexpr bool result6 = validate_rfc5892_context_single_pass(
        label6_arr.begin(), label6_arr.end());
    static_assert(result6 == true, "Test Case 6 Failed");
    std::cout << "Label 6: ";
    print_label(label6_arr.begin(), label6_arr.end());
    std::cout << "Validation Result: " << (result6 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 7: ZWNJ at beginning (Invalid - no valid context before)
    constexpr std::array<char32_t, 2> label7_arr = {ZWNJ, U'\u0644'};
    constexpr bool result7 = validate_rfc5892_context_single_pass(
        label7_arr.begin(), label7_arr.end());
    static_assert(result7 == false, "Test Case 7 Failed");
    std::cout << "Label 7: ";
    print_label(label7_arr.begin(), label7_arr.end());
    std::cout << "Validation Result: " << (result7 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 8: Label with no ZWJ/ZWNJ (Valid by default)
    constexpr std::array<char32_t, 3> label8_arr = {U'a', U'b', U'c'};
    constexpr bool result8 = validate_rfc5892_context_single_pass(
        label8_arr.begin(), label8_arr.end());
    static_assert(result8 == true, "Test Case 8 Failed");
    std::cout << "Label 8: ";
    print_label(label8_arr.begin(), label8_arr.end());
    std::cout << "Validation Result: " << (result8 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 9: ZWJ at beginning (Invalid)
    constexpr std::array<char32_t, 2> label9_arr = {ZWJ, U'a'};
    constexpr bool result9 = validate_rfc5892_context_single_pass(
        label9_arr.begin(), label9_arr.end());
    static_assert(result9 == false, "Test Case 9 Failed");
    std::cout << "Label 9: ";
    print_label(label9_arr.begin(), label9_arr.end());
    std::cout << "Validation Result: " << (result9 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example 10: ZWNJ at end (Invalid - no valid context after for regex)
    constexpr std::array<char32_t, 2> label10_arr = {U'\u0644',
                                                     ZWNJ}; // LAM (D) + ZWNJ
    constexpr bool result10 = validate_rfc5892_context_single_pass(
        label10_arr.begin(), label10_arr.end());
    static_assert(
        result10 == false,
        "Test Case 10 Failed"); // Fails regex check due to missing R/D after
    std::cout << "Label 10: ";
    print_label(label10_arr.begin(), label10_arr.end());
    std::cout << "Validation Result: " << (result10 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    // Example using std::u32string (requires C++11 or later)
    std::u32string label11_str = U"\u0644\u200C\u0627"; // Valid ZWNJ context
    bool result11 = validate_rfc5892_context_single_pass(label11_str.begin(),
                                                         label11_str.end());
    std::cout << "Label 11 (string): ";
    print_label(label11_str.begin(), label11_str.end());
    std::cout << "Validation Result: " << (result11 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    std::u32string label12_str = U"\u0627\u200C\u0644"; // Invalid ZWNJ context
    bool result12 = validate_rfc5892_context_single_pass(label12_str.begin(),
                                                         label12_str.end());
    std::cout << "Label 12 (string): ";
    print_label(label12_str.begin(), label12_str.end());
    std::cout << "Validation Result: " << (result12 ? "Valid" : "Invalid")
              << std::endl
              << std::endl;

    return 0;
}
