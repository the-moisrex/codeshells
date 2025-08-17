#include "../webpp/unicode/normalization.hpp"
#include "../webpp/unicode/unicode.hpp"
#include "./common/tests_common_pch.hpp"

using namespace webpp;
using namespace webpp::unicode;

// Trace what happens during this code:
// decompose_iterator decomp{spos, send};
// next_code_point(decomp, std::default_sentinel);
// prev_code_point(decomp, std::default_sentinel);
// When spos and send point to UTF-16 u"\x0230\x0330"sv string.

TEST(UnicodeDecomposeIterator, UTF16TraceTest) {
    // The UTF-16 string contains:
    // 0x0230 = LATIN CAPITAL LETTER O WITH DOT ABOVE
    // 0x0330 = COMBINING TILDE BELOW
    
    char16_t const str[] = {0x0230, 0x0330};
    std::u16string_view sv{str, 2};
    
    auto spos = sv.begin();
    auto send = sv.end();
    
    // Create the decompose iterator
    // This will decompose the first code point (0x0230) which is 'Ȯ'
    // According to UnicodeData.txt: 0230;LATIN CAPITAL LETTER O WITH DOT ABOVE;Lu;0;L;004F 0307;;;;N;;;;0231;
    // So 0x0230 decomposes to 0x004F (O) + 0x0307 (COMBINING DOT ABOVE)
    
    decompose_iterator decomp{spos, send};
    
    // Initially, the decompose_iterator should have decomposed 0x0230 into:
    // 1. 0x004F (O) - starter character
    // 2. 0x0307 (COMBINING DOT ABOVE) - combining mark
    
    // The buffer should contain these decomposed characters
    // *decomp should return the first character in the buffer (0x004F)
    EXPECT_EQ(*decomp, u'O');
    
    // After incrementing once, we should get the combining dot above
    ++decomp;
    EXPECT_EQ(*decomp, u'\x0307');
    
    // After incrementing again, we should get the original combining tilde below (0x0330)
    // This is because the decompose_iterator moves to the next code point in the original string
    // and decomposes it (but 0x0330 doesn't decompose further)
    ++decomp;
    EXPECT_EQ(*decomp, u'\x0330');
    
    // Now test going backwards
    // After decrementing, we should get combining dot above again
    --decomp;
    EXPECT_EQ(*decomp, u'\x0307');
    
    // After decrementing again, we should get 'O'
    --decomp;
    EXPECT_EQ(*decomp, u'O');
    
    // Test the checked::next_code_point and unchecked::prev_code_point functions
    {
        decompose_iterator decomp2{spos, send};
        
        // Get the first code point
        auto first_cp = checked::next_code_point(decomp2, std::default_sentinel);
        EXPECT_EQ(first_cp, U'O'); // 0x004F
        
        // Get the second code point
        auto second_cp = checked::next_code_point(decomp2, std::default_sentinel);
        EXPECT_EQ(second_cp, U'\x0307');
        
        // Get the third code point
        auto third_cp = checked::next_code_point(decomp2, std::default_sentinel);
        EXPECT_EQ(third_cp, U'\x0330');
        
        // Go back to the previous code point
        auto prev_cp = checked::prev_code_point(decomp2, std::default_sentinel);
        EXPECT_EQ(prev_cp, U'\x0330');
        
        // Go back again
        prev_cp = checked::prev_code_point(decomp2, std::default_sentinel);
        EXPECT_EQ(prev_cp, U'\x0307');
        
        // Go back again
        prev_cp = checked::prev_code_point(decomp2, std::default_sentinel);
        EXPECT_EQ(prev_cp, U'O');
    }
}

// Manual test without using utf32 iterators
TEST(UnicodeDecomposeIterator, ManualUTF16Test) {
    char16_t const str[] = {0x0230, 0x0330};
    std::u16string_view sv{str, 2};
    
    auto spos = sv.begin();
    auto send = sv.end();
    
    // Let's manually trace what happens:
    // 1. Create decompose_iterator with spos pointing to 0x0230
    // 2. The constructor calls canonical_decompose_to to decompose 0x0230
    
    decompose_iterator decomp{spos, send};
    
    // Check decomposition of 0x0230
    // From UnicodeData.txt: 0230;LATIN CAPITAL LETTER O WITH DOT ABOVE;Lu;0;L;004F 0307;;;;N;;;;0231;
    // So it decomposes to 'O' (0x004F) + COMBINING DOT ABOVE (0x0307)
    
    // The first character should be 'O'
    EXPECT_EQ(*decomp, u'O');
    
    // After one increment, we should get the combining mark
    ++decomp;
    EXPECT_EQ(*decomp, u'\x0307'); // COMBINING DOT ABOVE
    
    // After another increment, we should get the next code point from the original string
    // which is 0x0330 (COMBINING TILDE BELOW), which doesn't decompose further
    ++decomp;
    EXPECT_EQ(*decomp, u'\x0330'); // COMBINING TILDE BELOW
    
    // Test reverse iteration
    --decomp;
    EXPECT_EQ(*decomp, u'\x0307'); // COMBINING DOT ABOVE
    
    --decomp;
    EXPECT_EQ(*decomp, u'O'); // LATIN CAPITAL LETTER O
}
