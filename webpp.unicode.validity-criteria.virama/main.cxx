import standard;

#include <cstdint>
#include <print>

using namespace std;

constexpr static uint32_t virama[] = {
    0x094D,  0x09CD,  0x0A4D,  0x0ACD,  0x0B4D,  0x0BCD,  0x0C4D,  0x0CCD,
    0x0D3B,  0x0D3C,  0x0D4D,  0x0DCA,  0x0E3A,  0x0EBA,  0x0F84,  0x1039,
    0x103A,  0x1714,  0x1734,  0x17D2,  0x1A60,  0x1B44,  0x1BAA,  0x1BAB,
    0x1BF2,  0x1BF3,  0x2D7F,  0xA806,  0xA82C,  0xA8C4,  0xA953,  0xA9C0,
    0xAAF6,  0xABED,  0x10A3F, 0x11046, 0x1107F, 0x110B9, 0x11133, 0x11134,
    0x111C0, 0x11235, 0x112EA, 0x1134D, 0x11442, 0x114C2, 0x115BF, 0x1163F,
    0x116B6, 0x1172B, 0x11839, 0x1193D, 0x1193E, 0x119E0, 0x11A34, 0x11A47,
    0x11A99, 0x11C3F, 0x11D44, 0x11D45, 0x11D97};

int main() {

    println("Virama size: {}", sizeof(virama));
    println("Virama length: {}", sizeof(virama) / sizeof(uint32_t));

    /// These are only 45, so this is not how we get them:
    // 094D;DEVANAGARI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 09CD;BENGALI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0A4D;GURMUKHI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0ACD;GUJARATI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0B4D;ORIYA SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0BCD;TAMIL SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0C4D;TELUGU SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0CCD;KANNADA SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0D3B;MALAYALAM SIGN VERTICAL BAR VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0D3C;MALAYALAM SIGN CIRCULAR VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0D4D;MALAYALAM SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0EBA;LAO SIGN PALI VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 0F84;TIBETAN MARK HALANTA;Mn;9;NSM;;;;;N;TIBETAN VIRAMA;;;;
    // 1039;MYANMAR SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 1714;TAGALOG SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 1BAB;SUNDANESE SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // A8C4;SAURASHTRA SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // A8F3;DEVANAGARI SIGN CANDRABINDU VIRAMA;Lo;0;L;;;;;N;;;;;
    // A8F4;DEVANAGARI SIGN DOUBLE CANDRABINDU VIRAMA;Lo;0;L;;;;;N;;;;;
    // A953;REJANG VIRAMA;Mc;9;L;;;;;N;;;;;
    // AAF6;MEETEI MAYEK VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 10A3F;KHAROSHTHI VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 11046;BRAHMI VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 11070;BRAHMI SIGN OLD TAMIL VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 110B9;KAITHI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 11133;CHAKMA VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 111C0;SHARADA SIGN VIRAMA;Mc;9;L;;;;;N;;;;;
    // 11235;KHOJKI SIGN VIRAMA;Mc;9;L;;;;;N;;;;;
    // 112EA;KHUDAWADI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 1134D;GRANTHA SIGN VIRAMA;Mc;9;L;;;;;N;;;;;
    // 113CE;TULU-TIGALARI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 113CF;TULU-TIGALARI SIGN LOOPED VIRAMA;Mc;9;L;;;;;N;;;;;
    // 11442;NEWA SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 114C2;TIRHUTA SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 115BF;SIDDHAM SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 1163F;MODI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 116B6;TAKRI SIGN VIRAMA;Mc;9;L;;;;;N;;;;;
    // 11839;DOGRA SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 1193E;DIVES AKURU VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 119E0;NANDINAGARI SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 11A34;ZANABAZAR SQUARE SIGN VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 11C3F;BHAIKSUKI SIGN VIRAMA;Mn;9;L;;;;;N;;;;;
    // 11D45;MASARAM GONDI VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 11D97;GUNJALA GONDI VIRAMA;Mn;9;NSM;;;;;N;;;;;
    // 16D6B;KIRAT RAI SIGN VIRAMA;Lm;0;L;;;;;N;;;;;

    return 0;
}

// view ./build/optimizations.txt
