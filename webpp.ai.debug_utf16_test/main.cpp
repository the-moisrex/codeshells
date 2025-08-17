#include "../webpp/unicode/normalization.hpp"
#include "../webpp/unicode/unicode.hpp"
#include "./common/tests_common_pch.hpp"

using namespace webpp;
using namespace webpp::unicode;

// Let's debug what's happening with the decompose_iterator

TEST(UnicodeDecomposeIterator, DebugUTF16Test) {
    using std::u16string;

    // First, let's check what 0x0230 decomposes to
    {
        auto decomposed = canonical_decomposed<u16string>(U'\x0230');
        // According to UnicodeData.txt, 0230 should decompose to 004F 0307
        // which is 'O' + COMBINING DOT ABOVE
        EXPECT_EQ(decomposed.size(), 2);
        EXPECT_EQ(decomposed[0], u'O');         // 0x004F
        EXPECT_EQ(decomposed[1], u'\x0307');    // COMBINING DOT ABOVE
    }
    
    // Next, let's check what 0x0330 is
    {
        auto decomposed = canonical_decomposed<u16string>(U'\x0330');
        // 0x0330 is COMBINING TILDE BELOW and shouldn't decompose further
        EXPECT_EQ(decomposed.size(), 1);
        EXPECT_EQ(decomposed[0], u'\x0330');
    }
    
    // Now let's test with the actual UTF-16 string
    char16_t const str[] = {0x0230, 0x0330};
    std::u16string_view sv{str, 2};
    
    auto spos = sv.begin();
    auto send = sv.end();
    
    // Create the decompose iterator
    webpp::unicode::decompose_iterator decomp{spos, send};
    
    // Print what we actually get
    char16_t first = *decomp;
    ++decomp;
    char16_t second = *decomp;
    ++decomp;
    char16_t third = *decomp;
    
    // Let's check what these values actually are
    EXPECT_EQ(first, u'O');          // Should be 0x004F
    EXPECT_EQ(second, u'\x0307');    // Should be COMBINING DOT ABOVE
    EXPECT_EQ(third, u'\x0330');     // Should be COMBINING TILDE BELOW
}
