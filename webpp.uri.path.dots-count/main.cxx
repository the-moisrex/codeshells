import standard;

#include <cstdint>
#include <print>

namespace stl {
    using namespace std;
}

using namespace std;

namespace v1 {
    template <bool IgnoreWhitespace = true, typename IterT>
    [[nodiscard]] static constexpr stl::uint8_t dots_count(IterT pos,
                                                           IterT end) noexcept {
        using char_type = typename std::iterator_traits<IterT>::value_type;

        stl::uint8_t dots = 0;
        char_type prev = 0;

        for (;; ++pos) {
            if (pos == end) {
                break;
            }

            switch (*pos) {
            case '%':
                if (prev != 0) {
                    return 0;
                }
                prev = '%';
                continue;
            case '2':
                if (prev != '%') {
                    return 0;
                }
                prev = '2';
                continue;
            case 'e':
            case 'E':
                if (prev != '2') {
                    return 0;
                }
                prev = 0;
                ++dots;
                continue;
            case '.':
                ++dots;
                continue;
            [[unlikely]] case '\n':
            [[unlikely]] case '\r':
            [[unlikely]] case '\t':
                if constexpr (IgnoreWhitespace) {
                    continue;
                }
                [[fallthrough]];

            // a normal path:
            [[likely]] default:
                return 0;
            }
            break;
        }

        return dots;
    }
} // namespace v1

namespace v2 {

    // Character Category Lookup Table
    template <bool IgnoreWhitespaces = true>
    static constexpr auto dots_category = ([]() consteval {
        stl::array<stl::uint8_t, 256U> category = {};
        category.fill(5);  // Default category is 5 (other characters)
        category['%'] = 1; // Start of percent-encoding
        category['2'] = 2; // Part of %2e
        category['e'] = 3; // End of %2e (lowercase)
        category['E'] = 3; // End of %2E (uppercase)
        category['.'] = 4; // Literal dot
        if constexpr (IgnoreWhitespaces) {
            category['\n'] = 0; // Whitespace (ignored)
            category['\r'] = 0; // Whitespace (ignored)
            category['\t'] = 0; // Whitespace (ignored)
        }
        return category;
    })();

    // State Transition Table
    static constexpr stl::array<stl::array<stl::uint8_t, 6>, 11>
        state_transitions{{
            {0, 0, 0, 0, 0, 0}, // nothing
            // State 1: dot_count=1, matching=0 (valid state)
            {1, 6, 0, 0, 2, 0},
            // State 2: dot_count=2, matching=0 (valid state)
            {2, 8, 0, 0, 0, 0},

            // State 3: dot_count=0, matching=0 (initial state)
            {3, 4, 0, 0, 1, 0},
            // State 4: dot_count=0, matching=1 (after '%')
            {4, 0, 5, 0, 0, 0},
            // State 5: dot_count=0, matching=2 (after '%2')
            {5, 0, 0, 1, 0, 0},

            // State 6: dot_count=1, matching=1 (after '%')
            {6, 0, 7, 0, 0, 0},
            // State 7: dot_count=1, matching=2 (after '%2')
            {7, 0, 0, 2, 0, 0},

            // State 8: dot_count=2, matching=1 (after '%')
            {8, 0, 9, 0, 0, 0},
            // State 9: dot_count=2, matching=2 (after '%2')
            {9, 0, 0, 0, 0, 0},
            // State 10: invalid state
            {10, 0, 0, 0, 0, 0}}};

    template <bool IgnoreWhitespace = true, typename IterT>
    [[nodiscard]] static constexpr stl::uint8_t dots_count(IterT pos,
                                                           IterT end) noexcept {
        using char_type = typename std::iterator_traits<IterT>::value_type;

        char_type prev = 0;

        stl::uint8_t state = 3; // Start at initial state

        // **FSM Processing Loop**
        for (; pos != end; ++pos) {
            auto const category =
                dots_category<IgnoreWhitespace>[static_cast<unsigned char>(
                    *pos)];
            state = state_transitions[state][category];
            if (state == 0) {
                break; // Exit early if sequence becomes invalid
            }
        }

        if (state >= 3) {
            return 0;
        }
        return state;
    }

} // namespace v2


namespace v3 {

    // Character Category Lookup Table
    template <bool IgnoreWhitespaces = true>
    static constexpr auto dots_category = ([]() consteval {
        stl::array<stl::uint8_t, 256U> category = {};
        category.fill(5);  // Default category is 5 (other characters)
        category['%'] = 1; // Start of percent-encoding
        category['2'] = 2; // Part of %2e
        category['e'] = 3; // End of %2e (lowercase)
        category['E'] = 3; // End of %2E (uppercase)
        category['.'] = 4; // Literal dot
        if constexpr (IgnoreWhitespaces) {
            category['\n'] = 0; // Whitespace (ignored)
            category['\r'] = 0; // Whitespace (ignored)
            category['\t'] = 0; // Whitespace (ignored)
        }
        return category;
    })();

    // State Transition Table
    static constexpr stl::array<stl::array<stl::uint8_t, 6>, 5> // Reduced to 5 states
        state_transitions{{

            // State 0: Initial state / Invalid state (whitespace, %, 2, e|E, ., other)
            {0, 3, 0, 0, 1, 0}, // {0, 1, 0, 0, 1, 0}, // Modified: . -> state 1 (dot count 1), invalid -> 0

            // State 1: 1 Dot Found (final)
            {0, 0, 0, 0, 2, 0}, // {0, 0, 0, 0, 2, 0}, // Modified: '.' -> state 2 (dot count 2), invalid -> 0

            // State 2: 2 Dots Found (final)
            {0, 0, 0, 0, 0, 0}, // {0, 0, 0, 0, 0, 0}, // Modified: Invalid after 2 dots -> 0

            // State 3: After '%'
            {0, 0, 4, 0, 0, 0}, // {0, 0, 4, 0, 0, 0}, // Modified: '2' -> state 4. Invalid -> 0

            // State 4: After '%2'
            {0, 0, 0, 1, 0, 0}, // {0, 0, 0, 1, 0, 0}, // Modified: 'e'/'E' -> state 1 (1 dot). Invalid -> 0
    }};


    template <bool IgnoreWhitespace = true, typename IterT>
    [[nodiscard]] static constexpr stl::uint8_t dots_count(IterT pos,
                                                           IterT end) noexcept {
        using char_type = typename std::iterator_traits<IterT>::value_type;

        char_type prev = 0;

        stl::uint8_t state = 0; // Start at initial state

        // **FSM Processing Loop**
        for (; pos != end; ++pos) {
            auto const category =
                dots_category<IgnoreWhitespace>[static_cast<unsigned char>(
                    *pos)];
            state = state_transitions[state][category];
            if (state == 0) {
                break; // Exit early if sequence becomes invalid
            }
        }

        // state == 3 is 1 dot
        // state == 6 is 2 dots
        return state / 3;
    }

} // namespace v2
int index = 0;
bool failure = false;
void EXPECT_TRUE(bool val) {
    if (val) {
        println("\t: true");
    } else {
        println("\t: false");
        failure = true;
    }
    ++index;
}

void EXPECT_FALSE(bool val) { EXPECT_TRUE(!val); }

template <typename T, typename U>
void EXPECT_EQ(T lhs, U rhs) {
    if (lhs == rhs) {
        println("true:  {} == {}", lhs, rhs);
    } else {
        println("false: {} == {}", lhs, rhs);
        failure = true;
    }
    ++index;
}

int main() {

    auto const dots = [](std::string_view const input) {
        print("{}\t\t", input);
        return v2::dots_count<true>(input.begin(), input.end());
    };

    // --- Valid Dot Sequences ---
    EXPECT_EQ(dots("."), 1);       // Test 1: Valid_SingleDot
    EXPECT_EQ(dots(".."), 2);      // Test 2: Valid_DoubleDot
    EXPECT_EQ(dots("%2e"), 1);     // Test 3: Valid_Percent2e
    EXPECT_EQ(dots("%2E"), 1);     // Test 4: Valid_Percent2E
    EXPECT_EQ(dots("%2e%2e"), 2);  // Test 5: Valid_Percent2ePercent2e
    EXPECT_EQ(dots(".%2e"), 2);    // Test 6: Valid_DotPercent2e
    EXPECT_EQ(dots("%2e."), 2);    // Test 7: Valid_Percent2eDot
    EXPECT_EQ(dots("\n.\n"), 1);   // Test 8: Valid_DotWithWhitespace
    EXPECT_EQ(dots("\t..\t"), 2);  // Test 9: Valid_DoubleDotWithWhitespace
    EXPECT_EQ(dots("\r%2e\r"), 1); // Test 10: Valid_Percent2eWithWhitespace
    EXPECT_EQ(dots(".%2E"), 2);    // Test 11: Valid_MixedDotsAndPercent2e
    EXPECT_EQ(dots("%2E."), 2);    // Test 12: Valid_Percent2EDot

    // --- Invalid Dot Sequences ---
    EXPECT_EQ(dots("..."), 0); // Test 13: Invalid_TripleDot
    EXPECT_EQ(dots("%2e%2e%2e"),
              0);                // Test 14: Invalid_Percent2ePercent2ePercent2e
    EXPECT_EQ(dots(".%2e."), 0); // Test 15: Invalid_DotPercent2eDot
    EXPECT_EQ(dots("%"), 0);     // Test 16: Invalid_PartialPercent
    EXPECT_EQ(dots("%2"), 0);    // Test 17: Invalid_PartialPercent2
    EXPECT_EQ(dots("%2f"), 0);   // Test 18: Invalid_Percent2f
    EXPECT_EQ(dots("2e"), 0);    // Test 19: Invalid_2e
    EXPECT_EQ(dots("e2"), 0);    // Test 20: Invalid_e2
    EXPECT_EQ(dots(".a"), 0);    // Test 21: Invalid_DotFollowedByChar
    EXPECT_EQ(dots("..b"), 0);   // Test 22: Invalid_DoubleDotFollowedByChar
    EXPECT_EQ(dots("%2ec"), 0);  // Test 23: Invalid_Percent2eFollowedByChar
    EXPECT_EQ(dots("a."), 0);    // Test 24: Invalid_CharBeforeDot
    EXPECT_EQ(dots("b.."), 0);   // Test 25: Invalid_CharBeforeDoubleDot
    EXPECT_EQ(dots("c%2e"), 0);  // Test 26: Invalid_CharBeforePercent2e
    EXPECT_EQ(dots(".%"), 0);    // Test 27: Invalid_DotPercent
    EXPECT_EQ(dots("..%"), 0);   // Test 28: Invalid_DoubleDotPercent
    EXPECT_EQ(dots("%2e%"), 0);  // Test 29: Invalid_Percent2ePercent
    EXPECT_EQ(dots("%2e.."), 0); // Test 30: Invalid_Percent2eDoubleDot
    EXPECT_EQ(dots(".%2e.."), 0);      // Test 31: Invalid_DotPercent2eDoubleDot
    EXPECT_EQ(dots("..%2e."), 0);      // Test 32: Invalid_DoubleDotPercent2eDot
    EXPECT_EQ(dots("%2e\n.\t%2e"), 0); // Test 33: Invalid_WhitespaceInSequence
    EXPECT_EQ(dots(" ."), 0);          // Test 34: Invalid_LeadingSpace
    EXPECT_EQ(dots("  .."), 0);    // Test 35: Invalid_LeadingSpacesDoubleDot
    EXPECT_EQ(dots("   %2e"), 0);  // Test 36: Invalid_LeadingSpacesPercent2e
    EXPECT_EQ(dots(" \t\n\r"), 0); // Test 37: Invalid_OnlyWhitespace
    EXPECT_EQ(dots(""), 0);        // Test 38: Invalid_EmptyInput
    EXPECT_EQ(dots("...."), 0);    // Test 39: Invalid_QuadrupleDot
    EXPECT_EQ(dots("%2e%2e."), 0); // Test 40: Invalid_Percent2ePercent2eDot
    EXPECT_EQ(dots(".%2e%2e"), 0); // Test 41: Invalid_DotPercent2ePercent2e
    EXPECT_EQ(dots("abc."), 0);    // Test 42: Invalid_CharsBeforeDot
    EXPECT_EQ(dots("abc.."), 0);   // Test 43: Invalid_CharsBeforeDoubleDot
    EXPECT_EQ(dots("abc%2e"), 0);  // Test 44: Invalid_CharsBeforePercent2e
    EXPECT_EQ(dots("...%2e"), 0);  // Test 45: Invalid_TripleDotPercent2e
    EXPECT_EQ(dots("%2..."), 0);   // Test 46: Invalid_Percent2TripleDot
    EXPECT_EQ(dots("%2e.a"), 0);   // Test 47: Invalid_Percent2eDotChar
    EXPECT_EQ(dots("..a"), 0);     // Test 48: Invalid_DoubleDotChar
    EXPECT_EQ(dots(".a"), 0);      // Test 49: Invalid_SingleDotChar
    EXPECT_EQ(dots("   ."), 0);    // Test 50: Invalid_WhitespaceBeforeDot

    // --- Whitespace Handling Tests (NoWhitespace Options) ---
    EXPECT_EQ(dots("."), 1);     // Re-test valid dot without whitespace
    EXPECT_EQ(dots(".."), 2);    // Re-test valid double dot without whitespace
    EXPECT_EQ(dots("%2e"), 1);   // Re-test valid percent 2e without whitespace
    EXPECT_EQ(dots("\n.\n"), 1); // Test 51: NoWhitespace_Invalid_DotWithNewline
    EXPECT_EQ(dots("\t..\t"),
              2); // Test 52: NoWhitespace_Invalid_DoubleDotWithTab
    EXPECT_EQ(dots("\r%2e\r"),
              1); // Test 53: NoWhitespace_Invalid_Percent2eWithReturn
    EXPECT_EQ(dots(" ."), 0); // Test 54: NoWhitespace_Invalid_LeadingSpaceDot
    EXPECT_EQ(dots(" "), 0);  // Test 55: NoWhitespace_Invalid_OnlySpace

    if (failure) {
        println("Failed.");
        return 1;
    } else {
        println("Done.");
    }
    return 0;
}
